#pragma once

#include "Libraries.hpp"
#include "MyData.hpp"
#include "AgentConnection.hpp"
#include "GroupManager.hpp"



class Server1 {
public:
	Server1() = default;
	~Server1();


	INT StartServer();
	std::vector<AgentConnection*> GetAgentConnections() const;
	BOOL CreateListeningSocket(INT port, SOCKET& outSocket);
	BOOL ListenForTcpPort(INT nPort, SOCKET listeningSocket);
	VOID AddAgentConnection(SOCKET socket);
	std::vector<AgentConnection*>::iterator RemoveAgentConnection(std::vector<AgentConnection*>::iterator& connectionIterator);
	VOID PrintActiveAgentSockets();
	std::vector<AgentConnection*>::iterator FindConnectionFromSocketStr(std::string szSocket);
	VOID RemoveConnectionFromAllGroups(AgentConnection* conn);

	// Functions to handle user input
	VOID HandleUserInput();
	VOID UserCloseConnection(std::string szSocket);
	VOID UserRunCommand(const std::vector<std::string>& parameters);
	VOID UserRunCommandOnGroup(const std::vector<std::string>& arrParameters);
	VOID UserShowMan();

	// Functions to handle socket fd_sets:
	fd_set GetMasterSet();
	fd_set GetReadSet();
	VOID InitMasterSet();
	VOID SetReadSetAsMaster();
	VOID AddSocketToMaster(SOCKET socket);
	INT WaitForSocketRead();
	BOOL IsSocketInSet(SOCKET socket);
	VOID RemoveSocketFromSet(SOCKET socket);

private:
	std::vector<AgentConnection*> arrAgentConnections;
	std::vector<AgentConnection*> arrControllerConnections;
	fd_set masterSet;
	fd_set readSet;
	BOOL bIsRunning;
	GroupManager groupManager;

	mutable std::mutex mAgentConnectionsMutex;
	mutable std::mutex mControllerConnectionsMutex;
};

