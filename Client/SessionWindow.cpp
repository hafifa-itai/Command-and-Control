#include "SessionWindow.hpp"

SessionWindow::SessionWindow() {
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
	//DWORD bytesWritten;
	//std::string initialMsg = "C:\\>\n";
	//WriteConsoleA(hConsoleOut, initialMsg.c_str(), initialMsg.length(), NULL, NULL);
}

SessionWindow::~SessionWindow()
{
	CloseHandle(hConsoleOut);
	CloseHandle(hConsoleIn);
}

VOID SessionWindow::CommandLoop()
{
	std::string szCommand;
	std::string szResponse;
	CHAR carrCommand[512];
	CHAR carrResponse[4096];
	DWORD dwBytesRead;
	DWORD dwBytesWritten;

	while (TRUE) {
		// 1. Wait for and read the parent's message from the pipe.
		if (!ReadFile(hPipeFromParent, carrResponse, sizeof(carrResponse) - 1, &dwBytesRead, NULL) || dwBytesRead == 0) {
			break;
		}
		carrResponse[dwBytesRead] = '\0';

		// 2. Write the parent's message to the child's OWN screen.
		szResponse = std::string(carrResponse);
		WriteConsoleA(hConsoleOut, szResponse.c_str(), szResponse.length(), &dwBytesWritten, NULL);

		// 4. Read the response from the child's OWN keyboard.
		ReadConsoleA(hConsoleIn, carrCommand, sizeof(carrCommand) - 1, &dwBytesRead, NULL);
		// ReadConsoleA includes the \r\n, so we remove it.
		if (dwBytesRead >= 2) {
			carrCommand[dwBytesRead - 2] = '\0';

			szCommand = std::string(carrCommand);

			if (szCommand == "exit") {
				break;
			}
			// 5. Write the response back to the PARENT through the pipe.
			if (!WriteFile(hPipeToParent, szCommand.c_str(), szCommand.length(), &dwBytesWritten, NULL)) {
				break;
			}
		}
		else {
			exit(1);
		}
	}
}
