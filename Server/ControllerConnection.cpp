#include "ControllerConnection.hpp"

ControllerConnection::ControllerConnection(SOCKET controllerSocket)
    : Connection(controllerSocket) {
}


ControllerConnection::~ControllerConnection() {
}

BOOL ControllerConnection::SendData(const std::string& command) {
    if (socket == INVALID_SOCKET) return false;
    int sent = send(socket, command.c_str(), static_cast<int>(command.size()), 0);
    return sent == command.size();
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