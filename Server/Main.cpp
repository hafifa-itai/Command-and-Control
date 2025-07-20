#include "Server.hpp"

INT main() {
	try {
		Server server;
		server.StartServer();
	}
	catch (const std::exception& e) {
		std::cerr << "Fatal: " << e.what() << '\n';
	}

	return 0;
}