#include "Server1.hpp"

BOOL Server1::ListenForTcpPort(INT nPort, SOCKET listeningSocket)
{
    sockaddr_in clientAddr;
    INT nAddrLen = sizeof(clientAddr);
    SocketClientInfo socketClientInfo;

    fd_set master_set, read_set;
    FD_ZERO(&master_set);
    FD_SET(listeningSocket, &master_set);

    while (TRUE) {
        read_set = master_set;

        INT activity = select(0, &read_set, nullptr, nullptr, nullptr);
        if (activity == SOCKET_ERROR) {
            std::cerr << "select() failed\n";
            break;
        }

        // Check for new connections
        if (FD_ISSET(listeningSocket, &read_set)) {

            SOCKET clientSocket = accept(listeningSocket, (sockaddr*)&clientAddr, &nAddrLen);

            if (clientSocket != INVALID_SOCKET) {
                FD_SET(clientSocket, &master_set);
                AddAgentConnection(clientSocket);
            }
        }

        // Check for data or closed connections
        AgentConnection* connectionToRemove = NULL;
        std::unique_lock<std::mutex> lock(mAgentConnectionsMutex);

        for (auto connectionsIterator = arrAgentConnections.begin(); connectionsIterator != arrAgentConnections.end();) {
            INT nBytesReceived;
            AgentConnection* conn = *connectionsIterator;

            if (FD_ISSET(conn->GetSocket(), &read_set)) {
                std::string szData = conn->ReceiveData();

                if (szData.empty()) {
                    std::cout << "[-] Disonnected from " << conn->GetSocketStr() << "\n";
                    FD_CLR(conn->GetSocket(), &master_set);
                    connectionToRemove = conn;
                    connectionsIterator = RemoveAgentConnection(connectionsIterator, TRUE);
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
        Sleep(100);
    }

    return TRUE;
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
    arrThreads[1] = std::thread([this, arrListeningSockets] {
        this->ListenForTcpPort(3001, arrListeningSockets[1]);
        });

    std::cout << "[+] Listening for clients on port 3000\n";
    std::cout << "[+] Listening for agents on port 3001\n";
    std::cout << "[*] Command format: COMMNAD IP:PORT\n";
    std::cout << "[*] To close a connection, enter exit IP:PORT\n";

    while (TRUE) {
        BOOL bIsSocketFound = FALSE;
        std::string szIpPortToCommand;
        std::string szCommand;
        std::string szUserInput;

        std::getline(std::cin, szUserInput);
        std::istringstream iss(szUserInput);
        iss >> szCommand >> szIpPortToCommand;
        //std::cout << "command: " << szCommand << "\n";
        //std::cout << "ip and port: " << szIpPortToCommand << "\n";

        for (AgentConnection* conn : arrAgentConnections) {
            if (conn->GetSocketStr() == szIpPortToCommand) {
                //std::cout << "[*] Socket exists!\n";
                bIsSocketFound = TRUE;
                conn->SendCommand(szCommand);
            }
        }

        if (!bIsSocketFound) {
            std::cout << "[!] Socket does not exist!\n";
        }
    }

    arrThreads[1].join();

    closesocket(arrListeningSockets[0]);
    /*for (AgentConnection* conn : arrAgentConnections) {
        delete conn;
    }*/

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

std::vector<AgentConnection*> Server1::GetAgentConnections() const {
    std::lock_guard<std::mutex> lock(mAgentConnectionsMutex);
    return arrAgentConnections;
}

VOID Server1::AddAgentConnection(SOCKET socket) {
    std::lock_guard<std::mutex> lock(mAgentConnectionsMutex);
    AgentConnection* agentCon = new AgentConnection(socket);
    arrAgentConnections.push_back(agentCon);
    std::cout << "[+] Connected to " << agentCon->GetSocketStr() << "\n";
    PrintActiveAgentSockets();
}

std::vector<AgentConnection*>::iterator Server1::RemoveAgentConnection(std::vector<AgentConnection*>::iterator& connectionIterator, BOOL bIsLockHeld) {
    if (!bIsLockHeld) {
        std::lock_guard<std::mutex> lock(mAgentConnectionsMutex);
    }
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

//Server1::~Server1() {
//    for (AgentConnection* conn : arrAgentConnections) {
//        delete conn;
//    }
//}
