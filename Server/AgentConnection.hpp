#pragma once
#include "Connection.hpp"
#include "ThreadSafeQueue.hpp"

class AgentConnection : public Connection{
public:
    AgentConnection(SOCKET agentSocket);
    ~AgentConnection() override;

    INT GetSession();
    BOOL SendData(const std::wstring& wszCommand) override;
    BOOL ReceiveData(std::wstring& szoutBuffer) override;
    BOOL GetDataFromQueue(std::wstring& wszOutResponse, INT iTimeoutMs);
    VOID EnqueueIncomingData(const std::wstring& szData);
    VOID AddToGroup(std::wstring wszGroupName);
    VOID RemoveFromGroup(std::wstring wszGroupName);
    VOID SetSession(INT iNewSession);
    VOID SetHostName(std::wstring szNewHostName);
    std::wstring GetHostNameSessionStr();
    std::wstring GetHostName();
    std::vector<std::wstring> GetGroups();

private:
    INT iSession = 0;
    std::wstring wszHostname = L"";
    std::vector<std::wstring> arrGroups;
    ThreadSafeQueue qIncomingMessages;
};
