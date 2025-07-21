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
			return FALSE;
		}
	}
	return FALSE;
}

BOOL GroupManager::BroadcastToGroup(std::string szGroupName, std::string szCommand, std::string& szOutput)
{
	std::string szCwd;
	std::string szResponse;
	std::vector<std::string> arrConnectionsCwds;
	std::lock_guard<std::mutex> lock(mGroupMapMutex);

	if (groupMap.count(szGroupName)) {
		auto connectionsGroup = groupMap[szGroupName];
		if (connectionsGroup.size()) {
			for (AgentConnection* conn : connectionsGroup) {
				conn->SendData(szCommand);
				conn->GetDataFromQueue(szResponse, -1);
				ParseAgentResponse(szResponse, szCwd);
				arrConnectionsCwds.push_back("\\\\" + conn->GetHostNameSessionStr() + "\\" + szCwd + ">\n");

				if (!szResponse.empty()) {
					szResponse = "[*] received from " + conn->GetHostNameSessionStr() + " : \n" + szResponse + "\n";
					szOutput.append(szResponse);
				}
			}

			for (std::string szCurrentCwd : arrConnectionsCwds) {
				szOutput.append(szCurrentCwd);
			}

			return TRUE;
		}
		else {
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
				szOutput += "[*] IP: " + conn->GetSocketStr() + " | Host: " + conn->GetHostNameSessionStr() + "\n";
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
		szOutput = "[!] No groups found\n";
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

VOID GroupManager::ParseAgentResponse(std::string& szOutCleanedResp, std::string& szCwd)
{
	szCwd.clear();
	size_t last_char_pos = szOutCleanedResp.find_last_not_of(" \t\r\n");
	szOutCleanedResp.resize(last_char_pos + 1);
	size_t last_newline_pos = szOutCleanedResp.find_last_of('\n');

	if (last_newline_pos == std::string::npos) {
		szCwd = szOutCleanedResp;
		szOutCleanedResp.clear();
	}
	else {
		szCwd = szOutCleanedResp.substr(last_newline_pos + 1);
		szOutCleanedResp.erase(last_newline_pos);
		szOutCleanedResp.append("\n");
	}
}


