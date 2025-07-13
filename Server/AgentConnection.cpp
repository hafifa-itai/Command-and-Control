#include "AgentConnection.hpp"

AgentConnection::AgentConnection(SOCKET agentSocket)
    : Connection(agentSocket) {}


AgentConnection::~AgentConnection() {
}

BOOL AgentConnection::SendData(const std::string& command) {
    if (socket == INVALID_SOCKET) return false;
    int sent = send(socket, command.c_str(), static_cast<int>(command.size()), 0);
    return sent == command.size();
}

BOOL AgentConnection::ReceiveData(BOOL bIsPeekingData, std::string& szoutBuffer) {
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

