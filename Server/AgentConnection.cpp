#include "AgentConnection.hpp"

AgentConnection::AgentConnection(SOCKET agentSocket)
    : Connection(agentSocket) {}


AgentConnection::~AgentConnection() {
}

INT AgentConnection::GetSession()
{
    return iSession;
}


BOOL AgentConnection::SendData(const std::wstring& wszCommand) {
    INT iBytesSent;
    if (socket == INVALID_SOCKET) {
        return FALSE;
    }

    if (wszCommand.empty()) {
        iBytesSent = send(socket, reinterpret_cast<const CHAR*>(NOP_COMMAND), NOP_COMMAND_SIZE, 0);
        return iBytesSent == NOP_COMMAND_SIZE;
    }

    iBytesSent = send(socket, reinterpret_cast<const CHAR*>(wszCommand.c_str()), static_cast<int>(wszCommand.size() * sizeof(WCHAR)), 0);
    return iBytesSent == wszCommand.size() * sizeof(WCHAR);
}

BOOL AgentConnection::ReceiveData(std::wstring& szOutBuffer) {
    INT iBytesReceived;
    WCHAR wcarrBuffer[MAX_BUFFER_SIZE];

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
            iBytesReceived = recv(socket, reinterpret_cast<CHAR*>(wcarrBuffer), sizeof(wcarrBuffer) - sizeof(WCHAR), 0);
            wcarrBuffer[iBytesReceived / sizeof(WCHAR)] = '\0';
            szOutBuffer += wcarrBuffer;
            iTotalBytesReceived += iBytesReceived;
        }

        return TRUE; 
    }

    return FALSE;
}


BOOL AgentConnection::GetDataFromQueue(std::wstring& wszOutResponse, INT iTimeoutMs)
{
    return qIncomingMessages.WaitAndPop(wszOutResponse, iTimeoutMs);
}


VOID AgentConnection::EnqueueIncomingData(const std::wstring& wszData) {
    qIncomingMessages.Push(wszData);
}


VOID AgentConnection::AddToGroup(std::wstring wszGroupName) {
    arrGroups.push_back(wszGroupName);
}


VOID AgentConnection::RemoveFromGroup(std::wstring wszGroupName)
{
    auto groupsIterator = std::find(arrGroups.begin(), arrGroups.end(), wszGroupName);
    if (groupsIterator != arrGroups.end()) {
        arrGroups.erase(groupsIterator);
    }
}


VOID AgentConnection::SetSession(INT iNewSession)
{
    iSession = iNewSession;
}


VOID AgentConnection::SetHostName(std::wstring wszNewHostName)
{
    wszHostname = wszNewHostName;
}


std::wstring AgentConnection::GetHostNameSessionStr()
{
    return GetHostName() + L":" + std::to_wstring(GetSession());
}


std::wstring AgentConnection::GetHostName()
{
    return wszHostname;
}


std::vector<std::wstring> AgentConnection::GetGroups()
{
    return arrGroups;
}
