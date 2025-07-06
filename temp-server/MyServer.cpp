#include "Server.hpp"

//int main1() {
//    WSADATA wsaData;
//    SOCKET listenSocket = INVALID_SOCKET, clientSocket = INVALID_SOCKET;
//    sockaddr_in serverAddr = {}, clientAddr = {};
//    int clientAddrSize = sizeof(clientAddr);
//
//    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
//        std::cerr << "WSAStartup failed\n";
//        return 1;
//    }
//
//    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//    if (listenSocket == INVALID_SOCKET) {
//        std::cerr << "Socket creation failed\n";
//        WSACleanup();
//        return 1;
//    }
//
//    serverAddr.sin_family = AF_INET;
//    serverAddr.sin_port = htons(3001);
//    serverAddr.sin_addr.s_addr = INADDR_ANY;
//
//    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
//        std::cerr << "Bind failed\n";
//        closesocket(listenSocket);
//        WSACleanup();
//        return 1;
//    }
//
//    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
//        std::cerr << "Listen failed\n";
//        closesocket(listenSocket);
//        WSACleanup();
//        return 1;
//    }
//
//    std::cout << "[+] Server listening on port 3001...\n";
//
//    clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &clientAddrSize);
//    if (clientSocket == INVALID_SOCKET) {
//        std::cerr << "Accept failed\n";
//        closesocket(listenSocket);
//        WSACleanup();
//        return 1;
//    }
//
//    char buffer[1024] = {};
//    int received = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
//    if (received > 0) {
//        std::string data(buffer, received);
//        std::istringstream iss(data, std::ios::binary);
//        MyData myData;
//        myData.deserialize(iss);
//
//        std::cout << "[+] data.age = " << myData.age << "\n";
//        std::cout << "[+] data.name = " << myData.name << "\n";
//        std::cout << "[+] data.city = " << myData.city << "\n";
//    }
//    else {
//        std::cerr << "Receive failed or connection closed\n";
//    }
//
//    closesocket(clientSocket);
//    closesocket(listenSocket);
//    WSACleanup();
//    return 0;
//}

int main1() {
    HANDLE threads[2];
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    // Create listening socket for clients (port 3000)
    SOCKET clientListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in clientAddr = {};
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(3000);
    clientAddr.sin_addr.s_addr = INADDR_ANY;
    bind(clientListenSocket, (sockaddr*)&clientAddr, sizeof(clientAddr));
    listen(clientListenSocket, SOMAXCONN);

    // Create listening socket for agents (port 3001)
    SOCKET agentListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in agentAddr = {};
    agentAddr.sin_family = AF_INET;
    agentAddr.sin_port = htons(3001);
    agentAddr.sin_addr.s_addr = INADDR_ANY;
    bind(agentListenSocket, (sockaddr*)&agentAddr, sizeof(agentAddr));
    listen(agentListenSocket, SOMAXCONN);

    // Start listener threads
    threads[0] = CreateThread(nullptr, 0, ClientListener, (LPVOID)clientListenSocket, 0, nullptr);
    //threads[1] = CreateThread(nullptr, 0, AgentListener, (LPVOID)agentListenSocket, 0, nullptr);

    std::cout << "[+] Listening for clients on port 3000\n";
    std::cout << "[+] Listening for agents on port 3001\n";

    // Wait for threads (infinite)
    WaitForMultipleObjects(2, threads, TRUE, INFINITE);

    closesocket(clientListenSocket);
    closesocket(agentListenSocket);
    WSACleanup();
    return 0;
}
