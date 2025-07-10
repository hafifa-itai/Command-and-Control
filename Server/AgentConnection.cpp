#include "AgentConnection.hpp"

AgentConnection::AgentConnection(SOCKET agentSocket)
    : Connection(agentSocket) {}


AgentConnection::~AgentConnection() {
}

BOOL AgentConnection::SendCommand(const std::string& command) {
    if (socket == INVALID_SOCKET) return false;
    int sent = send(socket, command.c_str(), static_cast<int>(command.size()), 0);
    return sent == command.size();
}

std::string AgentConnection::ReceiveData() {
    if (socket == INVALID_SOCKET) return "";
    char buffer[4096];
    int received = recv(socket, buffer, sizeof(buffer) - 1, 0);

    if (received > 0) {
        buffer[received] = '\0';
        return std::string(buffer);
    }
    return "";
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

