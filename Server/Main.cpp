#include "Server1.hpp"

INT main() {
	try {
		Server1 server;
		server.StartServer();
	}
	catch (const std::exception& e) {
		std::cerr << "Fatal: " << e.what() << '\n';
	}

	return 0;
}