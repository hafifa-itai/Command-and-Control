#pragma once

#include "pch.h"
#include "AgentConnection.hpp"

class GroupManager
{
public:
	BOOL CreateGroup(std::string szGroupName);
	BOOL DeleteGroup(std::string szGroupName);
	BOOL BroadcastToGroup(std::string szGroupName, std::string szCommand, std::string& szOutput);
	BOOL ListGroupMembers(std::string szGroupName, std::string& szOutput);
	BOOL CheckGroupExists(std::string szGroupName);
	BOOL RemoveConnectionFromGroup(std::string szGroupName, AgentConnection* lpAgentConnection);
	VOID GetGroupNames(std::string& szOutput);
	VOID ParseAgentResponse(std::string& szOutCleanedResp, std::string& szCwd);
	VOID AddConnectionToGroup(std::string szGroupName, AgentConnection* lpAgentConnection);

private:
	std::mutex mGroupMapMutex;
	std::unordered_map<std::string, std::vector<AgentConnection*>> groupMap;
};

