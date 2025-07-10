#pragma once
#include "Connection.hpp"

class AgentConnection : public Connection{
public:
    AgentConnection(SOCKET agentSocket);
    ~AgentConnection() override;

    BOOL SendCommand(const std::string& command) override;
    std::string ReceiveData() override;

    VOID AddToGroup(std::string szGroupName);
    VOID RemoveFromGroup(std::string szGroupName);
    std::vector<std::string> GetGroups();
private:
    std::vector<std::string> arrGroups;
};
