#pragma once

#include "pch.h"

class SessionWindow
{
public:
	SessionWindow();
	~SessionWindow();

	VOID CommandLoop();

private:
	HANDLE hPipeFromParent;
	HANDLE hPipeToParent;

	HANDLE hConsoleOut;
	HANDLE hConsoleIn;
};

