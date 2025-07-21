#pragma once

#include "pch.h"
#include "AgentConnection.hpp"

class GroupManager
{
public:
	BOOL CreateGroup(std::wstring szGroupName);
	BOOL DeleteGroup(std::wstring wszGroupName);
	BOOL BroadcastToGroup(std::wstring wszGroupName, std::wstring szCommand, std::wstring& wszOutput);
	BOOL ListGroupMembers(std::wstring wszGroupName, std::wstring& wszOutput);
	BOOL CheckGroupExists(std::wstring wszGroupName);
	BOOL RemoveConnectionFromGroup(std::wstring wszGroupName, AgentConnection* lpAgentConnection);
	VOID GetGroupNames(std::wstring& wszOutput);
	VOID ParseAgentResponse(std::wstring& wszOutCleanedResp, std::wstring& wszCwd);
	VOID AddConnectionToGroup(std::wstring wszGroupName, AgentConnection* lpAgentConnection);

private:
	std::mutex mGroupMapMutex;
	std::unordered_map<std::wstring, std::vector<AgentConnection*>> groupMap;
};

