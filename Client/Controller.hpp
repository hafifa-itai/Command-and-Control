#pragma once

#include "pch.h"
#include "UserInputHandler.hpp"
#include "ControllerCommandReq.hpp"
#include "SessionWindow.hpp"
#define CHILD_PROCESS_FLAG "--child"

class Controller {
public:
	Controller(const std::string& szIp, INT iPort);
	~Controller();

	BOOL Connect();
	VOID Run();
	VOID HandleCommandObject(ControllerCommandReq commandReq);
	BOOL SendCommand(ControllerCommandReq commandReq);
	BOOL ReceiveData(std::string& szoutBuffer);
	VOID OpenSessionWindow(ControllerCommandReq commandReq,std::string szInitialCwd);
	BOOL ReadFromChild(HANDLE hChildStdoutRead, std::string& szoutCommand);
	BOOL WriteToChild(HANDLE hChildStdinWrite, const std::string& szData);
	

	// Execute command:
	VOID ShowMan();

private:
	INT iServerPort;
	BOOL bIsRunning;
	SOCKET sock;
	std::string szServerIp;
	UserInputHandler inputHandler;
	std::vector<std::thread> arrWindowSessionThreads;
};