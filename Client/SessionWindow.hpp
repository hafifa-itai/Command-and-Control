#pragma once

#include "pch.h"
#include "Constants.hpp"

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

