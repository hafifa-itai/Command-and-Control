#include "Connection.hpp"

VOID Connection::GetSocketClientInfo(SocketClientInfo& outSocketClientInfo) {
    CHAR carrIpStr[INET_ADDRSTRLEN];

    sockaddr_in addr;
    int addrSize = sizeof(addr);

    if (getpeername(socket, (sockaddr*)&addr, &addrSize) == 0) {
        inet_ntop(AF_INET, &(addr.sin_addr), carrIpStr, INET_ADDRSTRLEN);
        outSocketClientInfo.nPort = ntohs(addr.sin_port);
        outSocketClientInfo.szIp = std::string(carrIpStr);
    }
    else {
        outSocketClientInfo.nPort = 0;
        outSocketClientInfo.szIp = "";
    }
}