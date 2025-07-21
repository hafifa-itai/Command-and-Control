#pragma once

#include "pch.h"
#include "Constants.hpp"

class SessionWindow
{
public:
	SessionWindow();
	~SessionWindow();

	VOID GetUserCommands();
	VOID PrintParentMessage();

private:
	BOOL bIsRunning;
	HANDLE hPipeFromParent;
	HANDLE hPipeToParent;
	HANDLE hConsoleOut;
	HANDLE hConsoleIn;
};

