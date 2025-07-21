#pragma once

#include "Connection.hpp"
class ControllerConnection : public Connection
{
public:
    ControllerConnection(SOCKET controllerSocket);
    ~ControllerConnection() override;

    BOOL SendData(const std::wstring& wszCommand) override;
    BOOL ReceiveData(std::wstring& szoutBuffer) override;
};

