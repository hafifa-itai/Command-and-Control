#include "Server.hpp"


BOOL ListenForTcpPort(LPVOID lparam)
{
    sockaddr_in addr{};
    INT addrLen = sizeof(addr);
    ThreadArgs* lpThreadArgs = (ThreadArgs*)lparam;
    SOCKET clientSocket = accept(lpThreadArgs->sock, reinterpret_cast<sockaddr*>(&addr), &addrLen);

    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "accept() failed. Error: " << WSAGetLastError() << '\n';
        //closesocket(lpThreadArgs->sock);
        return FALSE;
    }

    // To Agent
    if (!lpThreadArgs->bIsToClient) {
        AgentConnection* agentCon = new AgentConnection(clientSocket);
        agentCon->SendCommand("echo hello");
        std::string resp = agentCon->ReceiveData();
        std::cout << "[+] Got response: " << resp << "\n";
        delete agentCon;
    }
    
    return TRUE;
}

BOOL CreateSocket(INT port, SOCKET& outSocket) {
    outSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (outSocket == INVALID_SOCKET)
        return false;

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


int main() {
    HANDLE threads[2];
    WSADATA wsaData;

    ThreadArgs* argsArray = new ThreadArgs[2];

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    CreateSocket(3000, argsArray[0].sock);
    CreateSocket(3001, argsArray[1].sock);

    argsArray[0].bIsToClient = TRUE;
    argsArray[1].bIsToClient = FALSE;

    // Start listener threads
    threads[0] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ListenForTcpPort , (LPVOID)&argsArray[0], 0, NULL);
    threads[1] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ListenForTcpPort, (LPVOID)&argsArray[1], 0, NULL);

    std::cout << "[+] Listening for clients on port 3000\n";
    std::cout << "[+] Listening for agents on port 3001\n";

    // Wait for threads (infinite)
    WaitForMultipleObjects(2, threads, TRUE, INFINITE);

    closesocket(argsArray[0].sock);
    closesocket(argsArray[1].sock);
    delete[] argsArray;
    WSACleanup();

    return 0;
}
