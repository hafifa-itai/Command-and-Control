#include "TcpServer.hpp"
#include <iostream>

int main() {
    try {
        TcpServer srv(3001);
        std::cout << "Type commands:\n"
            "  list          – show active clients\n"
            "  close <ip>    – drop connection by IP\n"
            "  quit          – stop server\n";
        srv.start();                    // blocking
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << '\n';
    }
    return 0;
}