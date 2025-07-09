#pragma once

#include "Libraries.hpp"
#include "MyData.hpp"
#include "AgentConnection.hpp"

#pragma comment(lib, "Ws2_32.lib")

class Server1 {
public:
	Server1() = default;
	~Server1();


	INT StartServer();
	std::vector<AgentConnection*> GetAgentConnections() const;
	BOOL ListenForTcpPort(INT nPort, SOCKET listeningSocket);
	VOID AddAgentConnection(SOCKET socket);
	std::vector<AgentConnection*>::iterator RemoveAgentConnection(std::vector<AgentConnection*>::iterator& connectionIterator);
	VOID PrintActiveAgentSockets();

	// Functions to handle socket fd_sets:
	fd_set GetMasterSet();
	fd_set GetReadSet();
	VOID InitMasterSet();
	VOID SetReadSetAsMaster();
	VOID AddSocketToMaster(SOCKET socket);
	INT WaitForSocketRead();
	BOOL IsSocketInSet(SOCKET socket);
	VOID RemoveSocketFromSet(SOCKET socket);

	static BOOL CreateListeningSocket(INT port, SOCKET& outSocket);

private:
	std::vector<AgentConnection*> arrAgentConnections;
	std::vector<AgentConnection*> arrControllerConnections;
	fd_set masterSet;
	fd_set readSet;
	BOOL bIsRunning;

	mutable std::mutex mAgentConnectionsMutex;
	mutable std::mutex mControllerConnectionsMutex;
};

