#include "Server1.hpp"


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

    while (bIsRunning) {
        BOOL bIsSocketFound = FALSE;
        std::string szIpPortToCommand;
        std::string szCommand;

        std::cout << "[*] Enter socket to control on [IP:PORT] -> ";
        std::getline(std::cin, szIpPortToCommand);
        std::unique_lock<std::mutex> lock(mAgentConnectionsMutex);

        for (auto connectionsIterator = arrAgentConnections.begin(); connectionsIterator != arrAgentConnections.end();) {
            AgentConnection* conn = *connectionsIterator;
            if (conn->GetSocketStr() == szIpPortToCommand) {

                std::cout << "[*] Enter command (exit to close the connection) -> ";
                std::getline(std::cin, szCommand);
                bIsSocketFound = TRUE;

                if (szCommand != "exit") {
                    conn->SendCommand(szCommand);
                }
                else {
                    connectionsIterator = RemoveAgentConnection(connectionsIterator);
                }

                break;
            }
            ++connectionsIterator;
        }
        
        lock.unlock();

        if (!bIsSocketFound) {
            if (szIpPortToCommand == "quit") {
                bIsRunning = FALSE;
            }
            else {
                std::cout << "[!] Socket does not exist!\n";
            }
        }
    }

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
                std::string szData = conn->ReceiveData();

                if (szData.empty()) {
                    std::cout << "[-] Disconnected from " << conn->GetSocketStr() << "\n";
                    RemoveSocketFromSet(conn->GetSocket());
                    connectionsIterator = RemoveAgentConnection(connectionsIterator);
                }
                else {
                    std::cout << "[+] Agent " << conn->GetSocketStr() << " response:\n" << szData << std::endl;
                    ++connectionsIterator;
                }
            }
            else {
                ++connectionsIterator;
            }
        }

        lock.unlock();
        //Sleep(100);
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


Server1::~Server1() {
    std::cout << "destructor!\n";
    for (AgentConnection* conn : arrAgentConnections) {
        delete conn;
    }
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
