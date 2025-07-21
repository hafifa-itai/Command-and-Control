#include "Controller.hpp"

INT main (INT iArgumentCount, CHAR* arrArgumentVector[]) {
	if (iArgumentCount < 2) {
		Controller controller(SERVER_IP, CONTROLLER_PORT);

		if (controller.Connect()) {
			controller.Run();
		}
	}
	else {
		SetConsoleTitleA(arrArgumentVector[1]);
		SessionWindow sessionWindow;
		std::thread readFromParentThread(&SessionWindow::PrintParentMessage, &sessionWindow);
		sessionWindow.GetUserCommands();
		readFromParentThread.join();
	}
}