#pragma once

#include "Libraries.hpp"
#include "UserInputHandler.hpp"
#include <nlohmann/json.hpp>

class Controller {
public:
	Controller(const std::string& szIp, INT iPort);
	~Controller();

	BOOL Connect();
	VOID Run();
	VOID HandleCommandObject(ControllerCommandReq commandReq);
	BOOL SendCommand(ControllerCommandReq commandReq);
	BOOL ReceiveData(std::string& szoutBuffer);

	// Execute command:
	VOID ShowMan();

private:
	INT iServerPort;
	BOOL bIsRunning;
	SOCKET sock;
	std::string szServerIp;
	UserInputHandler inputHandler;
};