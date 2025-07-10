#include "GroupManager.hpp"

BOOL GroupManager::CreateGroup(std::string szGroupName) {
	std::lock_guard<std::mutex> lock(mGroupMapMutex);
	auto result = groupMap.emplace(szGroupName, std::vector<AgentConnection*>());
	return result.second;
}


VOID GroupManager::AddConnectionToGroup(std::string szGroupName, AgentConnection* lpAgentConnection) {
	std::lock_guard<std::mutex> lock(mGroupMapMutex);
	groupMap[szGroupName].push_back(lpAgentConnection);
	lpAgentConnection->AddToGroup(szGroupName);
}

BOOL GroupManager::RemoveConnectionFromGroup(std::string szGroupName, AgentConnection* lpAgentConnection) {
	std::lock_guard<std::mutex> lock(mGroupMapMutex);

	if (groupMap.count(szGroupName)) {
		auto& connectionsGroup = groupMap[szGroupName];
		auto connectionIterator = std::find(connectionsGroup.begin(), connectionsGroup.end(), lpAgentConnection);

		if (connectionIterator != connectionsGroup.end()) {
			(*connectionIterator)->RemoveFromGroup(szGroupName);
			connectionsGroup.erase(connectionIterator);
			return TRUE;
		}
		else {
			std::cout << "Connection not found\n";
			return FALSE;
		}
	}
	return FALSE;
}

BOOL GroupManager::ListGroupMembers(std::string szGroupName) {
	std::lock_guard<std::mutex> lock(mGroupMapMutex);

	if (groupMap.count(szGroupName)) {
		auto connectionsGroup = groupMap[szGroupName];
		if (connectionsGroup.size()) {
			std::cout << "[*] Group " << szGroupName << " active connections:\n";
			for (AgentConnection* conn : connectionsGroup) {
				std::cout << "[*] " << conn->GetSocketStr() << "\n";
			}

			return TRUE;
		}
		else {
			std::cout << "[*] Group " << szGroupName << " is empty\n";
			return FALSE;
		}
	}

	return FALSE;
}

