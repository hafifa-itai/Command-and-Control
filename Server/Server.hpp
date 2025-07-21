#pragma once

#include "pch.h"
#include "Constants.hpp"
#include "ControllerConnection.hpp"
#include "AgentConnection.hpp"
#include "GroupManager.hpp"
#include "ControllerCommandReq.hpp"

class Server {
public:
	Server() = default;
	~Server();
	
	// General:
	INT StartServer();
	BOOL CreateListeningSocket(INT port, SOCKET& outSocket);
	BOOL ListenForConnections(INT nPort, SOCKET listeningSocket);
	VOID AcceptNewConnections(SOCKET listeningSocket, INT iFdSetIndex);
	VOID DeleteAgentConnectionsFiles();

	// Agent connections:
	INT AssignSession(std::string szHostName);
	BOOL CloseConnection(std::string szSocket);
	VOID CheckForAgentConnections();
	VOID AddAgentConnection(SOCKET socket);
	VOID RemoveConnectionFromAllGroups(AgentConnection* conn);
	std::string GetActiveAgentSockets();
	std::vector<AgentConnection*>::iterator RemoveAgentConnection(std::vector<AgentConnection*>::iterator& connectionIterator);
	std::vector<AgentConnection*>::iterator FindConnectionFromSocketStr(std::string szSocket);

	// Controller connections:
	VOID AddControllerConnection(SOCKET socket);
	VOID CheckForControllerConnections();
	VOID HandleControllerCommand(std::string szData,ControllerConnection* conn);
	std::vector<ControllerConnection*>::iterator RemoveControllerConnection(std::vector<ControllerConnection*>::iterator& connectionIterator);

	// Functions to handle socket fd_sets:
	INT WaitForSocketRead(INT iFdSetIndex);
	BOOL IsSocketInSet(SOCKET socket, INT iFdSetIndex);
	VOID InitMasterSet(INT iFdSetIndex);
	VOID SetReadSetAsMaster(INT iFdSetIndex);
	VOID AddSocketToMaster(SOCKET socket, INT iFdSetIndex);
	VOID RemoveSocketFromSet(SOCKET socket, INT iFdSetIndex);
	fd_set GetMasterSet(INT iFdSetIndex);
	fd_set GetReadSet(INT iFdSetIndex);

private:
	std::vector<AgentConnection*> arrAgentConnections;
	std::vector<ControllerConnection*> arrControllerConnections;
	fd_set masterSet[2];
	fd_set readSet[2];
	BOOL bIsRunning;
	GroupManager groupManager;

	mutable std::mutex mAgentConnectionsMutex;
	mutable std::mutex mControllerConnectionsMutex;
};

