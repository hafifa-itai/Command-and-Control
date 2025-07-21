#pragma once

#include "pch.h"
#include "ControllerCommandReq.hpp"
#include "SessionWindow.hpp"
#include "StringUtils.hpp"


class Controller {
public:
	Controller(const std::string& szIp, INT iPort);
	~Controller();

	VOID Run();
	VOID HandleCommandObject(ControllerCommandReq commandReq);
	VOID OpenSessionWindow(ControllerCommandReq commandReq);
	VOID ShowMan();
	BOOL Connect();
	BOOL SendCommand(ControllerCommandReq commandReq);
	BOOL ReceiveData(std::wstring& wszOutBuffer);
	BOOL WriteToChild(HANDLE hChildStdinWrite, const std::wstring& wszData);
	BOOL ReadFromChild(HANDLE hChildStdoutRead, std::wstring& wszOutCommand);
	CommandType StringToCommandType(const std::wstring& wszInput);
	ControllerCommandReq ValidateUserInput();

private:
	INT iServerPort;
	BOOL bIsRunning;
	SOCKET sock;
	HANDLE hJobHandle;
	std::string szServerIp;
	std::vector<std::thread> arrWindowSessionThreads;
};