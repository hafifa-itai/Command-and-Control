#include "Controller.hpp"

INT main(INT iArgumentCount, CHAR* arrArgumentVector[]) {
	if (iArgumentCount < 2) {
		Controller controller("127.0.0.1", CONTROLLER_PORT);

		if (controller.Connect()) {
			controller.Run();
		}
	}
	else {
		SetConsoleTitleA(arrArgumentVector[1]);
		SessionWindow sessionWindow;
		std::thread t(&SessionWindow::PrintParentMessage, &sessionWindow);
		sessionWindow.GetUserCommands();
		t.join();
	}
}