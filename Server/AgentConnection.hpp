#pragma once
#include "Connection.hpp"
#include "ThreadSafeQueue.hpp"

class AgentConnection : public Connection{
public:
    AgentConnection(SOCKET agentSocket);
    ~AgentConnection() override;

    INT GetSession();
    BOOL GetIsFileDeleted();
    BOOL SendData(const std::string& command) override;
    BOOL ReceiveData(std::string& szoutBuffer) override;
    BOOL GetDataFromQueue(std::string& szOutResponse, INT iTimeoutMs);
    VOID EnqueueIncomingData(const std::string& szData);
    VOID AddToGroup(std::string szGroupName);
    VOID RemoveFromGroup(std::string szGroupName);
    VOID SetSession(INT iNewSession);
    VOID SetHostName(std::string szNewHostName);
    VOID SetIsFileDeleted(BOOL bNewIsFileDeleted);
    std::string GetHostNameSessionStr();
    std::string GetHostName();
    std::vector<std::string> GetGroups();

private:
    INT iSession = 0;
    BOOL bIsFileDeleted;
    std::string szHostname = "";
    std::vector<std::string> arrGroups;
    ThreadSafeQueue qIncomingMessages;
};
