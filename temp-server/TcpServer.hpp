#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <string>
#include <thread>
#include <atomic>

#pragma comment(lib, "ws2_32.lib")

struct ClientInfo {
    SOCKET         sock;
    std::string    ip;
    uint16_t       port;
};

class TcpServer {
public:
    explicit TcpServer(uint16_t port = 3001);
    ~TcpServer();

    void start();                       // blocking – runs main loop
    void stop();                        // request graceful shutdown

private:
    void networkLoop();                 // select()-based loop
    void consoleThread();               // reads admin commands
    void acceptClient();
    void handleClient(size_t idx);
    void closeClient(size_t idx);
    void listClients() const;
    void processCommand(const std::string& line);

    SOCKET              listenSock_ = INVALID_SOCKET;
    fd_set              masterSet_{};
    std::vector<ClientInfo> clients_;
    std::thread         console_;
    std::atomic<bool>   running_{ false };
    uint16_t            port_;
};
