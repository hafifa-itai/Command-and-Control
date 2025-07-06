#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <conio.h>


#pragma comment(lib, "ws2_32.lib")

//
//DWORD WINAPI ClientListener(LPVOID lpParam) {
//    SOCKET listenSocket = (SOCKET)lpParam;
//    while (true) {
//        SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
//        if (clientSocket == INVALID_SOCKET) continue;
//
//        char buffer[4096];
//        int received = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
//        if (received > 0) {
//            buffer[received] = '\0';
//            std::cout << "[Client] Received: " << buffer << std::endl;
//        }
//        closesocket(clientSocket);
//    }
//    return 0;
//}
//
//
//DWORD WINAPI AgentListener(LPVOID lpParam) {
//    SOCKET listenSocket = (SOCKET)lpParam;
//    char response[4096];
//    while (true) {
//        SOCKET agentSocket = accept(listenSocket, nullptr, nullptr);
//        if (agentSocket == INVALID_SOCKET) continue;
//
//        const char* cmd = "echo hello";
//        send(agentSocket, cmd, (int)strlen(cmd), 0);
//        int len = recv(agentSocket, response, 4096, 0);
//        response[len] = '\0';
//        std::cout << "[+] output: " << response;
//        //closesocket(agentSocket);
//    }
//    return 0;
//}
//
//
//int main() {
//    HANDLE threads[2];
//    WSADATA wsaData;
//    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
//        std::cerr << "WSAStartup failed\n";
//        return 1;
//    }
//
//    // Create listening socket for clients (port 3000)
//    SOCKET clientListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//    sockaddr_in clientAddr = {};
//    clientAddr.sin_family = AF_INET;
//    clientAddr.sin_port = htons(3000);
//    clientAddr.sin_addr.s_addr = INADDR_ANY;
//    bind(clientListenSocket, (sockaddr*)&clientAddr, sizeof(clientAddr));
//    listen(clientListenSocket, SOMAXCONN);
//
//    // Create listening socket for agents (port 3001)
//    SOCKET agentListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//    sockaddr_in agentAddr = {};
//    agentAddr.sin_family = AF_INET;
//    agentAddr.sin_port = htons(3001);
//    agentAddr.sin_addr.s_addr = INADDR_ANY;
//    bind(agentListenSocket, (sockaddr*)&agentAddr, sizeof(agentAddr));
//    listen(agentListenSocket, SOMAXCONN);
//
//    // Start listener threads
//    threads[0] = CreateThread(nullptr, 0, ClientListener, (LPVOID)clientListenSocket, 0, nullptr);
//    threads[1] = CreateThread(nullptr, 0, AgentListener, (LPVOID)agentListenSocket, 0, nullptr);
//
//    std::cout << "[+] Listening for clients on port 3000\n";
//    std::cout << "[+] Listening for agents on port 3001\n";
//
//    // Wait for threads (infinite)
//    WaitForMultipleObjects(2, threads, TRUE, INFINITE);
//
//    closesocket(clientListenSocket);
//    closesocket(agentListenSocket);
//    WSACleanup();
//    return 0;
//}

std::string get_ip_string(const sockaddr_in& addr) {
    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), ipstr, sizeof(ipstr));
    return std::string(ipstr);
}

int main2() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(3001);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\n";
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed\n";
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "[+] Server listening on port 3001\n";

    // Store client sockets and their IP addresses
    std::vector<SOCKET> clientSockets;
    std::vector<std::string> clientIPs;

    fd_set master_set, read_set;
    FD_ZERO(&master_set);
    FD_SET(listenSocket, &master_set);

    while (true) {
        read_set = master_set;

        int activity = select(0, &read_set, nullptr, nullptr, nullptr);
        if (activity == SOCKET_ERROR) {
            std::cerr << "select() failed\n";
            break;
        }

        // Check for new connections
        if (FD_ISSET(listenSocket, &read_set)) {
            sockaddr_in clientAddr;
            int addrlen = sizeof(clientAddr);
            SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &addrlen);
            if (clientSocket != INVALID_SOCKET) {
                FD_SET(clientSocket, &master_set);
                clientSockets.push_back(clientSocket);
                clientIPs.push_back(get_ip_string(clientAddr));
                std::cout << "[+] New connection from " << clientIPs.back() << "\n";
            }
        }

        // Check for data or closed connections
        for (size_t i = 0; i < clientSockets.size(); ++i) {
            SOCKET s = clientSockets[i];
            if (FD_ISSET(s, &read_set)) {
                char buffer[1024];
                int received = recv(s, buffer, sizeof(buffer) - 1, 0);
                if (received <= 0) {
                    std::cout << "[-] Connection closed: " << clientIPs[i] << "\n";
                    closesocket(s);
                    FD_CLR(s, &master_set);
                    clientSockets.erase(clientSockets.begin() + i);
                    clientIPs.erase(clientIPs.begin() + i);
                    --i;
                }
                else {
                    buffer[received] = '\0';
                    std::cout << "[Client " << clientIPs[i] << "] " << buffer << std::endl;
                }
            }
        }

        // User input: close connection by IP
        if (_kbhit()) {
            std::string ipToClose;
            std::cout << "Enter IP to close: ";
            std::cin >> ipToClose;
            auto it = std::find(clientIPs.begin(), clientIPs.end(), ipToClose);
            if (it != clientIPs.end()) {
                size_t idx = std::distance(clientIPs.begin(), it);
                closesocket(clientSockets[idx]);
                FD_CLR(clientSockets[idx], &master_set);
                std::cout << "[*] Closed connection to " << ipToClose << "\n";
                clientSockets.erase(clientSockets.begin() + idx);
                clientIPs.erase(clientIPs.begin() + idx);
            }
            else {
                std::cout << "[!] No connection found for IP: " << ipToClose << "\n";
            }
        }
    }

    // Cleanup
    for (SOCKET s : clientSockets) closesocket(s);
    closesocket(listenSocket);
    WSACleanup();
    return 0;
}
