#pragma once

#include "pch.h"
#include "Constants.hpp"
#include "StringUtils.hpp"

typedef struct SocketClientInfo {
    std::wstring szIp;
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
    std::wstring GetSocketStr();

    virtual BOOL SendData(const std::wstring& command) = 0;
    virtual BOOL ReceiveData(std::wstring& szoutBuffer) = 0;

protected:
    SOCKET socket;
};



