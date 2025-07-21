#include "SessionWindow.hpp"

SessionWindow::SessionWindow() {
	bIsRunning = TRUE;

	// Handles to talk with Main Controller:
	hPipeFromParent = GetStdHandle(STD_INPUT_HANDLE);
	hPipeToParent = GetStdHandle(STD_OUTPUT_HANDLE);

	// Handles to interact with the user:
	hConsoleIn = CreateFileW(
		L"CONIN$",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	hConsoleOut = CreateFileW(
		L"CONOUT$",
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
	WCHAR wcarrResponse[MAX_BUFFER_SIZE];
	DWORD dwBytesWritten;
	DWORD dwBytesRead;
	DWORD dwBytesAvailable;

	while (bIsRunning) {

		if (!PeekNamedPipe(hPipeFromParent, NULL, 0, NULL, &dwBytesAvailable, NULL)) {
			bIsRunning = FALSE;
			break;
		}

		if (dwBytesAvailable > 0) {
			if (ReadFile(hPipeFromParent, wcarrResponse, sizeof(wcarrResponse), &dwBytesRead, NULL) && dwBytesRead > 0) {
				WriteConsoleW(hConsoleOut, wcarrResponse , dwBytesRead / sizeof(WCHAR), &dwBytesWritten, NULL);
				WriteConsoleW(hConsoleOut, L"> ", 2, &dwBytesWritten, NULL);
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
	WCHAR wcarrCommand[MAX_BUFFER_SIZE];
	DWORD dwBytesRead;
	DWORD dwBytesWritten;
	std::wstring wszCommand;

	while (bIsRunning) {
		ReadConsoleW(hConsoleIn, wcarrCommand, sizeof(wcarrCommand) / sizeof(WCHAR) - sizeof(WCHAR), &dwBytesRead, NULL);
		
		if (dwBytesRead > 2) {
			wcarrCommand[dwBytesRead - 2] = '\0';

			wszCommand = std::wstring(wcarrCommand);
			if (wszCommand == EXIT_COMMAND || wszCommand == QUIT_COMMAND) {
				bIsRunning = FALSE;
				break;
			}
			else if (!WriteFile(hPipeToParent, wszCommand.c_str(), wszCommand.length() * sizeof(WCHAR), &dwBytesWritten, NULL)) {
				bIsRunning = FALSE;
			}
		}
		else if (dwBytesRead != 2){
			bIsRunning = FALSE;
		}
		else {
			WriteConsoleW(hConsoleOut, L"> ", 2, &dwBytesWritten, NULL);
		}

	}
}
