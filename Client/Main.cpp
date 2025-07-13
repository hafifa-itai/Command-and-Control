#include "Controller.hpp"

INT main() {
	Controller controller("127.0.0.1", CONTROLLER_PORT);
	if (controller.Connect()) {
		controller.Run();
	}
}