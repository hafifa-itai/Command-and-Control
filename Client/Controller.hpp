#pragma once

#include "pch.h"
#include "ControllerCommandReq.hpp"
#include "SessionWindow.hpp"


class Controller {
public:
	Controller(const std::string& szIp, INT iPort);
	~Controller();

	BOOL Connect();
	BOOL SendCommand(ControllerCommandReq commandReq);
	BOOL ReceiveData(std::string& szoutBuffer);
	BOOL WriteToChild(HANDLE hChildStdinWrite, const std::string& szData);
	BOOL ReadFromChild(HANDLE hChildStdoutRead, std::string& szoutCommand);
	VOID Run();
	VOID HandleCommandObject(ControllerCommandReq commandReq);
	VOID OpenSessionWindow(ControllerCommandReq commandReq,std::string szInitialCwd);
	VOID ShowMan();
	CommandType StringToCommandType(const std::string& szInput);
	ControllerCommandReq ValidateUserInput();

private:
	INT iServerPort;
	BOOL bIsRunning;
	SOCKET sock;
	std::string szServerIp;
	std::vector<std::thread> arrWindowSessionThreads;
};