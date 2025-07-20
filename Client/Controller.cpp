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

        return FALSE;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(iServerPort);

    if (inet_pton(AF_INET, szServerIp.c_str(), &serverAddr.sin_addr) != 1) {
        std::cerr << "Invalid IP address\n";

        return FALSE;
    }

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection to server failed\n";

        return FALSE;
    }

    std::cout << "[+] Connected to " << szServerIp << ":" << iServerPort << "\n";
    return TRUE;
}

VOID Controller::Run()
{
    while (bIsRunning) {
        ControllerCommandReq commandReq = ValidateUserInput();
        HandleCommandObject(commandReq);
    }
}

ControllerCommandReq Controller::ValidateUserInput()
{
    CommandType commandType;
    std::string param;
    std::string szInput;
    std::string szCommand;
    ControllerCommandReq commandReq;
    std::vector<std::string> parameters;

    while (TRUE) {

        std::cout << "Enter Full command: ";
        std::getline(std::cin, szInput);

        std::istringstream iss(szInput);
        iss >> szCommand;

        while (iss >> param) {
            parameters.push_back(param);
        }

        for (const auto& p : parameters) {
            std::cout << "- " << p << "\n";
        }

        commandType = StringToCommandType(szCommand);

        switch (commandType) {
        case CommandType::Quit:
            return ControllerCommandReq(CommandType::Quit, "", "", "");

        case CommandType::Close:
            if (parameters.size() != 1) {
                std::cout << "[!] Invalid parameters for close command\n";
                break;
            }
            else {
                return ControllerCommandReq(CommandType::Close, parameters[0], "", "");
            }

        case CommandType::GroupCreate:
            if (parameters.size() == 1) {
                return ControllerCommandReq(CommandType::GroupCreate, "", parameters[0], "");
            }
            else {
                std::cout << "[!] Invalid parameters for group-create command\n";
                break;
            }
            

        case CommandType::GroupDelete:
            if (parameters.size() == 1) {
                return ControllerCommandReq(CommandType::GroupDelete, "", parameters[0], "");
            }
            else {
                std::cout << "[!] Invalid parameters for group-delete command\n";
                break;
            }

        case CommandType::GroupAdd:
            if (parameters.size() == 2) {
                return ControllerCommandReq(CommandType::GroupAdd, parameters[1], parameters[0], "");
            }
            else {
                std::cout << "[!] Invalid parameters for group-add command\n";
                break;
            }

        case CommandType::GroupRemove:
            if (parameters.size() == 2) {
                return ControllerCommandReq(CommandType::GroupRemove, parameters[1], parameters[0], "");
            }
            else {
                std::cout << "[!] Invalid parameters for group-remove command\n";
                break;
            }

        case CommandType::ListGroup:
            if (parameters.size() == 1) {
                return ControllerCommandReq(CommandType::ListGroup, "", parameters[0], "");
            }
            else {
                std::cout << "[!] Invalid parameters for group-list command\n";
                break;
            }

        case CommandType::ListGroupNames:
            return ControllerCommandReq(CommandType::ListGroupNames, "", "", "");

        case CommandType::List:
            return ControllerCommandReq(CommandType::List, "", "", "");

        case CommandType::Execute:
            if (parameters.size() != 1) {
                std::cout << "[!] Invalid parameters for cmd command\n";
                break;
            }
            else {
                return ControllerCommandReq(CommandType::OpenCmdWindow, parameters[0], "", "");
            }

        case CommandType::GroupExecute:
            if (parameters.size() != 1) {
                std::cout << "[!] Invalid parameters for group-cmd command\n";
                break;
            }
            else {
                return ControllerCommandReq(CommandType::OpenCmdWindow, "", parameters[0], "");
            }

        case CommandType::Man:
            return ControllerCommandReq(CommandType::Man, "", "", "");

        default:
            std::cout << "[!] Unrecognized command\n";
            break;
        }
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
    szCommand = j.dump(VERBOSE_JSON);
    iBytesSent = send(sock, szCommand.c_str(), static_cast<int>(szCommand.size()), 0);

    return iBytesSent == szCommand.size();
}

BOOL Controller::ReceiveData(std::string& szOutBuffer) {

    INT iBytesReceived;
    CHAR carrBuffer[MAX_BUFFER_SIZE];

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

        return TRUE;
    }

    return FALSE;
}

VOID Controller::OpenSessionWindow(ControllerCommandReq commandReq, std::string szInitialCwd)
{
    BOOL bIsGroupSession;
    BOOL bIsChildCreated;
    CHAR carrSelfPath[MAX_PATH];
    HANDLE hChildStdoutRead;
    HANDLE hChildStdoutWrite;
    HANDLE hChildStdinRead;
    HANDLE hChildStdinWrite;
    CommandType commandType;
    std::string szWindowName;
    std::string szWindowCommand;
    std::string szCommandOutput;
    std::string szProcessCommandLine;
    STARTUPINFOA siStartInfo{};
    SECURITY_ATTRIBUTES saAttr{};
    PROCESS_INFORMATION piProcInfo{};
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobInfo = {};

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    CreatePipe(&hChildStdoutRead, &hChildStdoutWrite, &saAttr, 0);
    SetHandleInformation(hChildStdoutRead, HANDLE_FLAG_INHERIT, 0);

    CreatePipe(&hChildStdinRead, &hChildStdinWrite, &saAttr, 0);
    SetHandleInformation(hChildStdinWrite, HANDLE_FLAG_INHERIT, 0);

    siStartInfo.cb = sizeof(STARTUPINFOA);
    siStartInfo.hStdError = hChildStdoutWrite;
    siStartInfo.hStdOutput = hChildStdoutWrite;
    siStartInfo.hStdInput = hChildStdinRead;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    if (commandReq.GetGroupName().empty()) {
        szWindowName = commandReq.GetTargetAgent();
        commandType = CommandType::Execute;
        bIsGroupSession = FALSE;
    }
    else {
        szWindowName = commandReq.GetGroupName();
        commandType = CommandType::GroupExecute;
        bIsGroupSession = TRUE;
    }

    GetModuleFileNameA(NULL, carrSelfPath, MAX_PATH);
    szProcessCommandLine = std::string(carrSelfPath) + " " + szWindowName;
    bIsChildCreated = CreateProcessA(
        NULL,
        &szProcessCommandLine[0],
        NULL,
        NULL,
        TRUE,
        CREATE_NEW_CONSOLE | CREATE_SUSPENDED,
        NULL,
        NULL,
        &siStartInfo,
        &piProcInfo
    );

    if (!bIsChildCreated) {
        std::cerr << "Failed to start PowerShell.\n";
        exit(1);
    }

    hJobHandle = CreateJobObject(NULL, NULL);
    jobInfo.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    SetInformationJobObject(hJobHandle, JobObjectExtendedLimitInformation, &jobInfo, sizeof(jobInfo));
    AssignProcessToJobObject(hJobHandle, piProcInfo.hProcess);
    ResumeThread(piProcInfo.hThread);

    // Close unused pipe ends
    CloseHandle(hChildStdoutWrite);
    CloseHandle(hChildStdinRead);

    // Get initial CWD
    SendCommand(ControllerCommandReq(commandType, commandReq.GetTargetAgent(), commandReq.GetGroupName(), ""));
    ReceiveData(szCommandOutput);
    WriteToChild(hChildStdinWrite, szCommandOutput);

    while (bIsRunning) {
        if (!ReadFromChild(hChildStdoutRead, szWindowCommand)) {
            break;
        }

        SendCommand(ControllerCommandReq(commandType, commandReq.GetTargetAgent(), commandReq.GetGroupName(), szWindowCommand));
        ReceiveData(szCommandOutput);

        if (!WriteToChild(hChildStdinWrite, szCommandOutput)) {
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

CommandType Controller::StringToCommandType(const std::string& szInput) {
    auto it = StringToCommandTypeMap.find(szInput);
    if (it != StringToCommandTypeMap.end()) {
        return it->second;
    }
    return CommandType::Unknown;
}

