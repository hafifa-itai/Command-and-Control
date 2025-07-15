#include "Controller.hpp"


Controller::Controller(const std::string& szIp, INT iPort) {
    szServerIp = szIp;
    iServerPort = iPort;
    bIsRunning = TRUE;
    sock = INVALID_SOCKET;
}


Controller::~Controller() {
    if (sock != INVALID_SOCKET) {
        closesocket(sock);
    }
    WSACleanup();
}


BOOL Controller::Connect()
{
    WSADATA wsaData;
    sockaddr_in serverAddr{};

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";

        return FALSE;
    }

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();

        return FALSE;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(iServerPort);

    if (inet_pton(AF_INET, szServerIp.c_str(), &serverAddr.sin_addr) != 1) {
        std::cerr << "Invalid IP address\n";
        closesocket(sock);
        WSACleanup();

        return FALSE;
    }

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection to server failed\n";
        closesocket(sock);
        WSACleanup();

        return FALSE;
    }

    std::cout << "[+] Connected to " << szServerIp << ":" << iServerPort << "\n";
    return TRUE;
}

VOID Controller::Run()
{
    while (bIsRunning) {
        ControllerCommandReq commandReq = inputHandler.CreateCommandObject();
        HandleCommandObject(commandReq);
    }
}

VOID Controller::HandleCommandObject(ControllerCommandReq commandReq)
{
    std::string szOutputBuffer;
    CommandType commandType = commandReq.GetCommandType();

    if (commandType == CommandType::Close || commandType == CommandType::List || commandType == CommandType::GroupAdd ||
        commandType == CommandType::GroupRemove || commandType == CommandType::GroupCreate || commandType == CommandType::GroupDelete ||
        commandType == CommandType::ListGroup || commandType == CommandType::ListGroupNames || commandType == CommandType::Quit) 
    {
        SendCommand(commandReq);
        ReceiveData(szOutputBuffer);
        std::cout << szOutputBuffer;
    }

    if (commandType == CommandType::Unknown) {
        std::cout << "[!] Unrecognized command!\n";
        ShowMan();
    }

    if (commandType == CommandType::Man) {
        ShowMan();
    }
}

BOOL Controller::SendCommand(ControllerCommandReq commandReq)
{
    INT iBytesSent;
    nlohmann::json j = commandReq;
    std::string szCommand;

    if (sock == INVALID_SOCKET) {
        return FALSE;
    }
    szCommand = j.dump(4);
    //std::cout << szCommand << "\n";
    iBytesSent = send(sock, szCommand.c_str(), static_cast<int>(szCommand.size()), 0);
    return iBytesSent == szCommand.size();
}

BOOL Controller::ReceiveData(std::string& szOutBuffer) {
    INT iBytesReceived;
    CHAR carrBuffer[4096];

    if (sock == INVALID_SOCKET) {
        return FALSE;
    }

    iBytesReceived = recv(sock, carrBuffer, sizeof(carrBuffer) - 1, 0);

    if (iBytesReceived > 0) {
        carrBuffer[iBytesReceived] = '\0';
        szOutBuffer = carrBuffer;
    }

    return FALSE;
 }


VOID Controller::ShowMan()
{
    std::cout << "[*] quit - Kill server and all connections\n";
    std::cout << "[*] close IP:PORT - Close connection with IP:PORT\n";
    std::cout << "[*] cmd COMMAND IP:PORT - Execute COMMAND on IP:PORT\n";
    std::cout << "[*] list - Show all active connections\n";
    std::cout << "[*] group-create - Create a new control group\n";
    std::cout << "[*] group-delete GROUPNAME - Delete GROUPNAME control group\n";
    std::cout << "[*] group-add GROUPNAME IP:PORT - Add IP:PORT to GROUPNAME control group\n";
    std::cout << "[*] group-remove GROUPNAME IP:PORT - Remove IP:PORT from GROUPNAME control group\n";
    std::cout << "[*] group-list GROUPNAME - Print the members of GROUPNAME control group\n";
    std::cout << "[*] group-cmd GROUPNAME COMMAND - Execute COMMAND on members of GROUPNAME control group\n";
    std::cout << "[*] groups - Print all active control groups\n";
    std::cout << "[*] man - Show this man page\n";
}
