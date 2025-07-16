#pragma once

#include "pch.h"
#include "AgentConnection.hpp"

class GroupManager
{
public:
	BOOL CreateGroup(std::string szGroupName);
	BOOL DeleteGroup(std::string szGroupName);
	VOID AddConnectionToGroup(std::string szGroupName, AgentConnection* lpAgentConnection);
	BOOL RemoveConnectionFromGroup(std::string szGroupName, AgentConnection* lpAgentConnection);
	BOOL BroadcastToGroup(std::string szGroupName, std::string szCommand);
	BOOL ListGroupMembers(std::string szGroupName, std::string& szOutput);
	VOID GetGroupNames(std::string& szOutput);
	BOOL CheckGroupExists(std::string szGroupName);

private:
	std::mutex mGroupMapMutex;
	std::unordered_map<std::string, std::vector<AgentConnection*>> groupMap;
};

