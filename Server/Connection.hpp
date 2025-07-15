#pragma once

#include "pch.h"

typedef struct SocketClientInfo {
    std::string szIp;
    INT nPort;
} SocketClientInfo;


class Connection {
public:
    Connection(SOCKET s) : socket (s) {}

    virtual ~Connection() {
        if (socket != INVALID_SOCKET) {
            closesocket(socket);
        }
    }

    BOOL IsConnectionAlive() const {
        return socket != INVALID_SOCKET;
    }
    
    SOCKET GetSocket() const {
        return socket;
    }

    VOID GetSocketClientInfo(SocketClientInfo& outSocketClientInfo);
    std::string GetSocketStr();

    virtual BOOL SendData(const std::string& command) = 0;
    virtual BOOL ReceiveData(BOOL bIsPeekingData, std::string& szoutBuffer) = 0;

protected:
    SOCKET socket;
};



