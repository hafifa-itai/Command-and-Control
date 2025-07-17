#include "AgentConnection.hpp"

AgentConnection::AgentConnection(SOCKET agentSocket)
    : Connection(agentSocket) {}


AgentConnection::~AgentConnection() {
}

BOOL AgentConnection::SendData(const std::string& command) {
    if (socket == INVALID_SOCKET) {
        return FALSE;
    }

    if (command.empty()) {
        INT sent = send(socket, NOP_COMMAND, NOP_COMMAND_SIZE, 0);
        return sent == NOP_COMMAND_SIZE;
    }

    INT sent = send(socket, command.c_str(), static_cast<int>(command.size()), 0);
    return sent == command.size();
}

BOOL AgentConnection::ReceiveData(std::string& szOutBuffer) {
    INT iBytesReceived;
    CHAR carrBuffer[4096];

    if (socket == INVALID_SOCKET) {
        return FALSE;
    }

    uint32_t uiNetMessageLen;
    uint32_t uiHostMessageLen;
    std::cout << "\n*before recv*\n";
    iBytesReceived = recv(socket, (LPSTR)&uiNetMessageLen, sizeof(uiNetMessageLen), 0);
    std::cout << "\n*after recv*\n";


    if (iBytesReceived > 0) {
        
        szOutBuffer.clear();
        uiHostMessageLen = ntohl(uiNetMessageLen);
        INT iTotalBytesReceived = 0;

        if (uiHostMessageLen > 20 * 1024 * 1024) {
            return FALSE;
        }

        while (iTotalBytesReceived < uiHostMessageLen) {
            iBytesReceived = recv(socket, carrBuffer, sizeof(carrBuffer) - 1, 0);
            carrBuffer[iBytesReceived] = '\0';
            szOutBuffer += carrBuffer;
            iTotalBytesReceived += iBytesReceived;
        }

        std::cout << szOutBuffer;
        return TRUE;
        
    }

    return FALSE;
}

BOOL AgentConnection::GetDataFromQueue(std::string& szOutResponse, INT iTimeoutMs)
{
    return qIncomingMessages.WaitAndPop(szOutResponse, iTimeoutMs);
}


VOID AgentConnection::EnqueueIncomingData(const std::string& szData) {
    qIncomingMessages.Push(szData);
}


VOID AgentConnection::AddToGroup(std::string szGroupName) {
    arrGroups.push_back(szGroupName);
}

VOID AgentConnection::RemoveFromGroup(std::string szGroupName)
{
    auto groupsIterator = std::find(arrGroups.begin(), arrGroups.end(), szGroupName);
    if (groupsIterator != arrGroups.end()) {
        arrGroups.erase(groupsIterator);
    }
}

std::vector<std::string> AgentConnection::GetGroups()
{
    return arrGroups;
}


//BOOL AgentConnection::IsConnectionAlive() const {
//    return socket != INVALID_SOCKET;
//}

