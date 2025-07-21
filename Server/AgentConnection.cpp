#include "AgentConnection.hpp"

AgentConnection::AgentConnection(SOCKET agentSocket)
    : Connection(agentSocket) {}


AgentConnection::~AgentConnection() {
}

INT AgentConnection::GetSession()
{
    return iSession;
}


BOOL AgentConnection::SendData(const std::string& command) {
    INT iBytesSent;
    if (socket == INVALID_SOCKET) {
        return FALSE;
    }

    if (command.empty()) {
        iBytesSent = send(socket, NOP_COMMAND, NOP_COMMAND_SIZE, 0);
        return iBytesSent == NOP_COMMAND_SIZE;
    }

    iBytesSent = send(socket, command.c_str(), static_cast<int>(command.size()), 0);
    return iBytesSent == command.size();
}

BOOL AgentConnection::ReceiveData(std::string& szOutBuffer) {
    INT iBytesReceived;
    CHAR carrBuffer[MAX_BUFFER_SIZE];

    if (socket == INVALID_SOCKET) {
        return FALSE;
    }

    uint32_t uiNetMessageLen;
    uint32_t uiHostMessageLen;
    iBytesReceived = recv(socket, (LPSTR)&uiNetMessageLen, sizeof(uiNetMessageLen), 0);

    if (iBytesReceived > 0) {
        
        szOutBuffer.clear();
        uiHostMessageLen = ntohl(uiNetMessageLen);
        INT iTotalBytesReceived = 0;

        if (uiHostMessageLen > MAX_MSG_SIZE) {
            return FALSE;
        }

        while (iTotalBytesReceived < uiHostMessageLen) {
            iBytesReceived = recv(socket, carrBuffer, sizeof(carrBuffer) - 1, 0);
            carrBuffer[iBytesReceived] = '\0';
            szOutBuffer += carrBuffer;
            iTotalBytesReceived += iBytesReceived;
        }

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


VOID AgentConnection::SetSession(INT iNewSession)
{
    iSession = iNewSession;
}


VOID AgentConnection::SetHostName(std::string szNewHostName)
{
    szHostname = szNewHostName;
}


std::string AgentConnection::GetHostNameSessionStr()
{
    return GetHostName() + ":" + std::to_string(GetSession());
}


std::string AgentConnection::GetHostName()
{
    return szHostname;
}


std::vector<std::string> AgentConnection::GetGroups()
{
    return arrGroups;
}
