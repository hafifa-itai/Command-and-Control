#pragma once

#include "Libraries.hpp"
#include "MyData.hpp"
#include "AgentConnection.hpp"

#pragma comment(lib, "Ws2_32.lib")

//class Server {
//public:
//	Server() = default;
//	~Server();
//
//
//	INT StartServer();
//
//	std::vector<AgentConnection*> GetAgentConnections() const {
//		std::lock_guard<std::mutex> lock(mAgentConnectionsMutex);
//		return arrAgentConnections;
//	}
//
//private:
//	std::vector<AgentConnection*> arrAgentConnections;
//	std::vector<AgentConnection*> arrControllerConnections;
//
//	mutable std::mutex mAgentConnectionsMutex;
//	mutable std::mutex mControllerConnectionsMutex;
//};
