#include "ControllerConnection.hpp"

ControllerConnection::ControllerConnection(SOCKET controllerSocket)
    : Connection(controllerSocket) {
}


ControllerConnection::~ControllerConnection() {
}

BOOL ControllerConnection::SendData(const std::wstring& wszCommand) {
    INT iBytesSent;
    std::wstring wszFinalData;

    if (socket == INVALID_SOCKET) {
        return FALSE;
    }

    uint32_t uiNetMessageLen = htonl(wszCommand.size() * sizeof(WCHAR));
    wszFinalData = std::wstring(reinterpret_cast<WCHAR*>(&uiNetMessageLen), sizeof(uint32_t) / sizeof(WCHAR));
    wszFinalData += wszCommand;
    iBytesSent = send(socket, reinterpret_cast<const CHAR*>(wszFinalData.data()), wszFinalData.length() * sizeof(WCHAR), 0);

    return iBytesSent == wszFinalData.size() * sizeof(WCHAR);
}

BOOL ControllerConnection::ReceiveData(std::wstring& wszOutBuffer) {
    INT iBytesReceived;
    WCHAR wcarrBuffer[MAX_BUFFER_SIZE];

    if (socket == INVALID_SOCKET) {
        return FALSE;
    }

    iBytesReceived = recv(socket, reinterpret_cast<CHAR*>(wcarrBuffer), sizeof(wcarrBuffer) - sizeof(WCHAR), 0);

    if (iBytesReceived > 0) {
        wcarrBuffer[iBytesReceived / sizeof(WCHAR)] = '\0';
        wszOutBuffer = wcarrBuffer;
        
        return TRUE;
    }

    return FALSE;
}