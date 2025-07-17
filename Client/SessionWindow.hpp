#pragma once

#include "pch.h"

class SessionWindow
{
public:
	SessionWindow();
	~SessionWindow();

	VOID PrintParentMessage();
	VOID GetUserCommands();

private:
	BOOL bIsRunning;
	HANDLE hPipeFromParent;
	HANDLE hPipeToParent;

	HANDLE hConsoleOut;
	HANDLE hConsoleIn;
};

