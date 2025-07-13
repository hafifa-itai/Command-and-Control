#pragma once
#include "Connection.hpp"

class AgentConnection : public Connection{
public:
    AgentConnection(SOCKET agentSocket);
    ~AgentConnection() override;

    BOOL SendData(const std::string& command) override;
    BOOL ReceiveData(BOOL bIsPeekingData, std::string& szoutBuffer) override;

    VOID AddToGroup(std::string szGroupName);
    VOID RemoveFromGroup(std::string szGroupName);
    std::vector<std::string> GetGroups();
private:
    std::vector<std::string> arrGroups;
};
