#pragma once

#include "Libraries.hpp"
#include "AgentConnection.hpp"

class GroupManager
{
public:
	BOOL CreateGroup(std::string szGroupName);
	VOID DeleteGroup(std::string szGroupName);
	VOID AddConnectionToGroup(std::string szGroupName, AgentConnection* lpAgentConnection);
	BOOL RemoveConnectionFromGroup(std::string szGroupName, AgentConnection* lpAgentConnection);
	VOID BroadcastToGroup(std::string szCommand);

private:
	std::mutex mGroupMapMutex;
	std::unordered_map<std::string, std::vector<AgentConnection*>> groupMap;
};

