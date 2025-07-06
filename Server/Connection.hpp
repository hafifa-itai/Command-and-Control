#pragma once
#include "Server.hpp"

class Connection {
public:
    Connection(SOCKET s) : socket (s) {}
    virtual ~Connection() {
        if (socket != INVALID_SOCKET) {
            closesocket(socket);
        }
    }

    bool IsConnectionAlive() const {
        return socket != INVALID_SOCKET;
    }

    virtual BOOL SendCommand(const std::string& command) = 0;
    virtual std::string ReceiveData() = 0;

protected:
    SOCKET socket;
};


