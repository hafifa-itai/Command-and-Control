#include "GroupManager.hpp"

BOOL GroupManager::CreateGroup(std::string szGroupName) {
	std::lock_guard<std::mutex> lock(mGroupMapMutex);
	auto result = groupMap.emplace(szGroupName, std::vector<AgentConnection*>());
	return result.second;
}


VOID GroupManager::AddConnectionToGroup(std::string szGroupName, AgentConnection* lpAgentConnection) {
	std::lock_guard<std::mutex> lock(mGroupMapMutex);
	groupMap[szGroupName].push_back(lpAgentConnection);
}

BOOL GroupManager::RemoveConnectionFromGroup(std::string szGroupName, AgentConnection* lpAgentConnection) {
	std::lock_guard<std::mutex> lock(mGroupMapMutex);
	if (groupMap.count(szGroupName)) {
		auto& connectionGroup = groupMap[szGroupName];
		auto connectionIterator = std::find(connectionGroup.begin(), connectionGroup.end(), lpAgentConnection);

		if (connectionIterator != connectionGroup.end()) {
			connectionGroup.erase(connectionIterator);
			return TRUE;
		}
		else {
			std::cout << "Connection not found\n";
			return FALSE;
		}
	}
	return TRUE;
}