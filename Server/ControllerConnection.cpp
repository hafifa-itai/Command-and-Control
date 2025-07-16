#include "ControllerConnection.hpp"

ControllerConnection::ControllerConnection(SOCKET controllerSocket)
    : Connection(controllerSocket) {
}


ControllerConnection::~ControllerConnection() {
}

BOOL ControllerConnection::SendData(const std::string& command) {
    INT iBytesSent;
    std::string szFinalData;

    if (socket == INVALID_SOCKET) {
        return false;
    }

    uint32_t uiNetMessageLen = htonl(command.size());
    szFinalData = std::string(reinterpret_cast<char*>(&uiNetMessageLen), sizeof(uint32_t));
    szFinalData += command;
    iBytesSent = send(socket, szFinalData.data(), szFinalData.length(), 0);

    return iBytesSent == szFinalData.size();
}

BOOL ControllerConnection::ReceiveData(BOOL bIsPeekingData, std::string& szoutBuffer) {
    INT iBytesReceived;
    CHAR carrBuffer[4096];

    if (socket == INVALID_SOCKET) {
        return FALSE;
    }

    iBytesReceived = recv(socket, carrBuffer, sizeof(carrBuffer) - 1, MSG_PEEK & bIsPeekingData);

    if (iBytesReceived > 0) {
        if (!bIsPeekingData) {
            carrBuffer[iBytesReceived] = '\0';
            szoutBuffer = carrBuffer;
        }

        return TRUE;
    }

    return FALSE;
}