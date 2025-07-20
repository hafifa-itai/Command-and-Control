#include "SessionWindow.hpp"

SessionWindow::SessionWindow() {
	bIsRunning = TRUE;

	// Handles to talk with Main Controller:
	hPipeFromParent = GetStdHandle(STD_INPUT_HANDLE);
	hPipeToParent = GetStdHandle(STD_OUTPUT_HANDLE);

	// Handles to interact with the user:
	hConsoleIn = CreateFileA(
		"CONIN$",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	hConsoleOut = CreateFileA(
		"CONOUT$",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
}

SessionWindow::~SessionWindow()
{
	CloseHandle(hConsoleOut);
	CloseHandle(hConsoleIn);
}

VOID SessionWindow::PrintParentMessage() {
	CHAR carrResponse[4096];
	DWORD dwBytesWritten;
	DWORD dwBytesRead;
	DWORD dwBytesAvailable;

	while (bIsRunning) {

		if (!PeekNamedPipe(hPipeFromParent, NULL, 0, NULL, &dwBytesAvailable, NULL)) {
			bIsRunning = FALSE;
			break;
		}

		if (dwBytesAvailable > 0) {
			if (ReadFile(hPipeFromParent, carrResponse, sizeof(carrResponse), &dwBytesRead, NULL) && dwBytesRead > 0) {
				WriteConsoleA(hConsoleOut, carrResponse , dwBytesRead, &dwBytesWritten, NULL);
				WriteConsoleA(hConsoleOut, "> ", 2, &dwBytesWritten, NULL);
			}
			else {
				bIsRunning = FALSE;
				break;
			}
		}
		else {
			Sleep(10);
		}
	}
}

VOID SessionWindow::GetUserCommands()
{
	BOOL bIsNewLine;
	CHAR carrCommand[512];
	DWORD dwBytesRead;
	DWORD dwBytesWritten;
	std::string szCommand;

	while (bIsRunning) {
		WriteConsoleA(hConsoleOut, "> ", 2, &dwBytesWritten, NULL);
		ReadConsoleA(hConsoleIn, carrCommand, sizeof(carrCommand) - 1, &dwBytesRead, NULL);

		if (dwBytesRead > 2) {
			carrCommand[dwBytesRead - 2] = '\0';

			szCommand = std::string(carrCommand);

			if (szCommand == "exit") {
				bIsRunning = FALSE;
				break;
			}
			if (!WriteFile(hPipeToParent, szCommand.c_str(), szCommand.length(), &dwBytesWritten, NULL)) {
				bIsRunning = FALSE;			}
		}
		else if (dwBytesRead != 2){
			bIsRunning = FALSE;
		}
	}
}
