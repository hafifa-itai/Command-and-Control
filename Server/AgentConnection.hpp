#pragma once
#include "Connection.hpp"
#include "ThreadSafeQueue.hpp"

#define NOP_COMMAND "$null = $null"
#define NOP_COMMAND_SIZE 13



class AgentConnection : public Connection{
public:
    AgentConnection(SOCKET agentSocket);
    ~AgentConnection() override;

    BOOL SendData(const std::string& command) override;
    BOOL ReceiveData(std::string& szoutBuffer) override;
    BOOL GetDataFromQueue(std::string& szOutResponse, INT iTimeoutMs);
    VOID EnqueueIncomingData(const std::string& szData);

    VOID AddToGroup(std::string szGroupName);
    VOID RemoveFromGroup(std::string szGroupName);
    std::vector<std::string> GetGroups();
private:
    std::vector<std::string> arrGroups;
    ThreadSafeQueue qIncomingMessages;
};
