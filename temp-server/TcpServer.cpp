#include "TcpServer.hpp"
#include <iostream>
#include <algorithm>

static std::string ipFromSockaddr(const sockaddr_in& sa) {
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &sa.sin_addr, buf, sizeof(buf));
    return buf;
}

TcpServer::TcpServer(uint16_t port) : port_(port) {
    WSADATA w;
    if (WSAStartup(MAKEWORD(2, 2), &w) != 0)
        throw std::runtime_error("WSAStartup failed");

    listenSock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSock_ == INVALID_SOCKET)
        throw std::runtime_error("socket() failed");

    sockaddr_in addr{}; addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(listenSock_, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR ||
        listen(listenSock_, SOMAXCONN) == SOCKET_ERROR)
        throw std::runtime_error("bind()/listen() failed");

    FD_ZERO(&masterSet_);
    FD_SET(listenSock_, &masterSet_);
}

TcpServer::~TcpServer() {
    stop();
    for (auto& c : clients_) closesocket(c.sock);
    if (listenSock_ != INVALID_SOCKET) closesocket(listenSock_);
    WSACleanup();
}

void TcpServer::start() {
    running_ = true;
    console_ = std::thread(&TcpServer::consoleThread, this);
    networkLoop();                      // blocks here
    console_.join();
}

void TcpServer::stop() { running_ = false; }

void TcpServer::networkLoop() {
    while (running_) {
        fd_set readSet = masterSet_;
        if (select(0, &readSet, nullptr, nullptr, nullptr) == SOCKET_ERROR) break;

        if (FD_ISSET(listenSock_, &readSet)) acceptClient();

        for (size_t i = 0; i < clients_.size(); ++i) {
            SOCKET s = clients_[i].sock;
            if (FD_ISSET(s, &readSet)) handleClient(i);
        }
    }
}

void TcpServer::acceptClient() {
    sockaddr_in sa; int len = sizeof(sa);
    SOCKET cs = accept(listenSock_, (sockaddr*)&sa, &len);
    if (cs == INVALID_SOCKET) return;

    ClientInfo info{ cs, ipFromSockaddr(sa), ntohs(sa.sin_port) };
    clients_.push_back(info);
    FD_SET(cs, &masterSet_);
    std::cout << "[+] Connected: " << info.ip << ':' << info.port << '\n';
}

void TcpServer::handleClient(size_t idx) {
    char buf[1024];
    int r = recv(clients_[idx].sock, buf, sizeof(buf) - 1, 0);
    if (r <= 0) {                       // closed or error
        std::cout << "[-] Disconnected: " << clients_[idx].ip << '\n';
        closeClient(idx);
    }
    else {
        buf[r] = 0;
        std::cout << '[' << clients_[idx].ip << "] " << buf << '\n';
    }
}

void TcpServer::closeClient(size_t idx) {
    FD_CLR(clients_[idx].sock, &masterSet_);
    closesocket(clients_[idx].sock);
    clients_.erase(clients_.begin() + idx);
}

void TcpServer::listClients() const {
    if (clients_.empty()) { std::cout << "(none)\n"; return; }
    for (const auto& c : clients_)
        std::cout << c.ip << ':' << c.port << '\n';
}

void TcpServer::processCommand(const std::string& line) {
    if (line == "quit") { stop(); return; }

    if (line == "list") { listClients(); return; }

    if (line.rfind("close ", 0) == 0) {
        std::string ip = line.substr(6);
        auto it = std::find_if(clients_.begin(), clients_.end(),
            [&](const ClientInfo& c) { return c.ip == ip; });
        if (it != clients_.end()) {
            size_t idx = std::distance(clients_.begin(), it);
            std::cout << "[*] Closing " << ip << '\n';
            closeClient(idx);
        }
        else std::cout << "No such IP\n";
        return;
    }
    std::cout << "Commands: list | close <ip> | quit\n";
}

void TcpServer::consoleThread() {       // runs in parallel with select()
    std::string line;
    while (running_ && std::getline(std::cin, line))
        processCommand(line);
}
