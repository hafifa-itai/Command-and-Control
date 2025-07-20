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
	

	INT StartServer();
	BOOL CreateListeningSocket(INT port, SOCKET& outSocket);
	BOOL ListenForConnections(INT nPort, SOCKET listeningSocket);
	VOID AcceptNewConnections(SOCKET listeningSocket, INT iFdSetIndex);
	std::vector<AgentConnection*>::iterator FindConnectionFromSocketStr(std::string szSocket);

	// Agent connections:
	VOID CheckForClosedAgentConnections();
	VOID AddAgentConnection(SOCKET socket);
	std::vector<AgentConnection*>::iterator RemoveAgentConnection(std::vector<AgentConnection*>::iterator& connectionIterator);
	VOID RemoveConnectionFromAllGroups(AgentConnection* conn);
	std::string GetActiveAgentSockets();


	// Controller connections:
	VOID AddControllerConnection(SOCKET socket);
	VOID CheckForControllerConnections();
	std::vector<ControllerConnection*>::iterator RemoveControllerConnection(std::vector<ControllerConnection*>::iterator& connectionIterator);
	VOID HandleControllerCommand(std::string szData,ControllerConnection* conn);



	// Functions to handle controller input
	//VOID HandleUserInput();
	BOOL CloseConnection(std::string szSocket);
	//VOID UserRunCommand(const std::vector<std::string>& parameters);
	//VOID UserRunCommandOnGroup(const std::vector<std::string>& arrParameters);
	//VOID UserShowMan();

	// Functions to handle socket fd_sets:
	fd_set GetMasterSet(INT iFdSetIndex);
	fd_set GetReadSet(INT iFdSetIndex);
	VOID InitMasterSet(INT iFdSetIndex);
	VOID SetReadSetAsMaster(INT iFdSetIndex);
	VOID AddSocketToMaster(SOCKET socket, INT iFdSetIndex);
	INT WaitForSocketRead(INT iFdSetIndex);
	BOOL IsSocketInSet(SOCKET socket, INT iFdSetIndex);
	VOID RemoveSocketFromSet(SOCKET socket, INT iFdSetIndex);

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

