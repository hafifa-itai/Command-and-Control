#pragma once
#include "Connection.hpp"

class AgentConnection : public Connection{
public:
    AgentConnection(SOCKET agentSocket);
    ~AgentConnection() override;

    BOOL SendCommand(const std::string& command) override;
    std::string ReceiveData() override;

};
