#include "GroupManager.hpp"

BOOL GroupManager::CreateGroup(std::string szGroupName) {
	std::lock_guard<std::mutex> lock(mGroupMapMutex);
	auto result = groupMap.emplace(szGroupName, std::vector<AgentConnection*>());
	return result.second;
}

BOOL GroupManager::DeleteGroup(std::string szGroupName)
{
	std::lock_guard<std::mutex> lock(mGroupMapMutex);

	if (groupMap.count(szGroupName)) {
		auto& connectionsGroup = groupMap[szGroupName];
		
		for (AgentConnection* conn : connectionsGroup) {
			conn->RemoveFromGroup(szGroupName);
		}

		groupMap.erase(szGroupName);
		return TRUE;
	}

	return FALSE;
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

BOOL GroupManager::BroadcastToGroup(std::string szGroupName, std::string szCommand)
{
	std::string szData;
	std::lock_guard<std::mutex> lock(mGroupMapMutex);

	if (groupMap.count(szGroupName)) {
		auto connectionsGroup = groupMap[szGroupName];
		if (connectionsGroup.size()) {
			for (AgentConnection* conn : connectionsGroup) {
				conn->SendData(szCommand);
				//conn->ReceiveData(szData);
				std::cout << "[*] received from " << conn->GetSocketStr() << " : \n" << szData << "\n";
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

BOOL GroupManager::ListGroupMembers(std::string szGroupName, std::string& szOutput) {
	std::lock_guard<std::mutex> lock(mGroupMapMutex);

	if (groupMap.count(szGroupName)) {
		auto connectionsGroup = groupMap[szGroupName];

		if (connectionsGroup.size()) {
			szOutput += "[*] Group " + szGroupName + " active connections:\n";
			for (AgentConnection* conn : connectionsGroup) {
				szOutput += "[*] " + conn->GetSocketStr() + "\n";
			}
		}
		else {
			szOutput = "[*] Group " + szGroupName + " is empty\n";
		}
		
		return TRUE;
	}

	szOutput = "[!] Group " + szGroupName + " doesn't exist\n";
	return FALSE;
}

VOID GroupManager::GetGroupNames(std::string& szOutput)
{
	std::lock_guard<std::mutex> lock(mGroupMapMutex);
	if (groupMap.empty()) {
		szOutput = "[!] No groups found";
	}
	else {
		for (const auto& keyPair : groupMap) {
			szOutput += "[*] " + keyPair.first + "\n";
		}
	}
}

BOOL GroupManager::CheckGroupExists(std::string szGroupName)
{
	if (groupMap.count(szGroupName)) {
		if (groupMap[szGroupName].size() > 0) {
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

