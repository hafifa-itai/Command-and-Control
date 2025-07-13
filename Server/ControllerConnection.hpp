#pragma once

#include "Connection.hpp"
class ControllerConnection : public Connection
{
public:
    ControllerConnection(SOCKET controllerSocket);
    ~ControllerConnection() override;

    BOOL SendData(const std::string& command) override;
    BOOL ReceiveData(BOOL bIsPeekingData, std::string& szoutBuffer) override;
};

