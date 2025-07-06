#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include "MyData.hpp"

#pragma comment(lib, "Ws2_32.lib")

int main() {
    WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
    sockaddr_in serverAddr = {};

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
    serverAddr.sin_port = htons(3000);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    MyData d("Itai","Bnei Darom", 20);
    std::ostringstream oss(std::ios::binary);
    d.serialize(oss);
    std::string buffer = oss.str();
    std::cout << "Serialized data size: " << buffer.size() << " bytes" << std::endl;

    int sent = send(sock, buffer.c_str(), buffer.size(), 0);
    if (sent == SOCKET_ERROR) {
        std::cerr << "Send failed\n";
    }
    else {
        for (unsigned char c : buffer) {
            printf("%02x", c);
        }
        std::cout << std::endl;
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
