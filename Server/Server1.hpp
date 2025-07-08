#pragma once

#include "Libraries.hpp"
#include "MyData.hpp"
#include "AgentConnection.hpp"

#pragma comment(lib, "Ws2_32.lib")

class Server1 {
public:
	Server1() = default;
	~Server1() {
		std::cout << "destructor!\n";
		for (AgentConnection* conn : arrAgentConnections) {
			delete conn;
		}
	}


	INT StartServer();
	std::vector<AgentConnection*> GetAgentConnections() const;
	BOOL ListenForTcpPort(INT nPort, SOCKET listeningSocket);
	VOID AddAgentConnection(SOCKET socket);
	std::vector<AgentConnection*>::iterator RemoveAgentConnection(std::vector<AgentConnection*>::iterator& connectionIterator, BOOL bIsLockHeld);
	VOID PrintActiveAgentSockets();

	static BOOL CreateListeningSocket(INT port, SOCKET& outSocket);

private:
	std::vector<AgentConnection*> arrAgentConnections;
	std::vector<AgentConnection*> arrControllerConnections;

	mutable std::mutex mAgentConnectionsMutex;
	mutable std::mutex mControllerConnectionsMutex;
};

