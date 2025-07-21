#include "PowerShellSession.hpp"



PowerShellSession::PowerShellSession()
{
    BOOL bIsPsCreated;
    HANDLE hChildStdoutWrite;
    HANDLE hChildStdinRead;
    HANDLE hChildStdinWriteTmp;
    HANDLE hChildStdoutReadTmp;
    STARTUPINFOA siStartInfo{};
    SECURITY_ATTRIBUTES saAttr{};

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    CreatePipe(&hChildStdoutReadTmp, &hChildStdoutWrite, &saAttr, 0);
    SetHandleInformation(hChildStdoutReadTmp, HANDLE_FLAG_INHERIT, 0);

    CreatePipe(&hChildStdinRead, &hChildStdinWriteTmp, &saAttr, 0);
    SetHandleInformation(hChildStdinWriteTmp, HANDLE_FLAG_INHERIT, 0);

    siStartInfo.cb = sizeof(STARTUPINFOA);
    siStartInfo.hStdError = hChildStdoutWrite;
    siStartInfo.hStdOutput = hChildStdoutWrite;
    siStartInfo.hStdInput = hChildStdinRead;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    CHAR carrCommand[] = "powershell.exe -NoLogo -NoExit -Command -";
    bIsPsCreated = CreateProcessA(
        NULL,
        carrCommand,
        NULL,
        NULL, TRUE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &siStartInfo,
        &piProcInfo
    );

    if (!bIsPsCreated) {
        std::cerr << "Failed to start PowerShell.\n";
        exit(1);
    }

    CloseHandle(hChildStdoutWrite);
    CloseHandle(hChildStdinRead);

    hChildStdoutRead = hChildStdoutReadTmp;
    hChildStdinWrite = hChildStdinWriteTmp;
}

PowerShellSession::~PowerShellSession()
{
    if (hChildStdoutRead) {
        CloseHandle(hChildStdoutRead);
    }

    if (hChildStdinWrite) {
        CloseHandle(hChildStdinWrite);
    }

    if (piProcInfo.hProcess) {
        TerminateProcess(piProcInfo.hProcess, 0);
        CloseHandle(piProcInfo.hProcess);
    }
    if (piProcInfo.hThread) {
        CloseHandle(piProcInfo.hThread);
    }
}

std::string PowerShellSession::generateUniqueMarker() {
    static std::random_device rndRandomSeed;
    static std::mt19937_64 generator(rndRandomSeed());
    std::uniform_int_distribution<unsigned long long> randomessDistribution;
    std::string szRandomStr = std::to_string(randomessDistribution(generator));
    std::string szUniqueTimeStamp = std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count());

    return "END_OF_CMD_MARKER_" + szRandomStr + "_" + szUniqueTimeStamp;
}


std::string PowerShellSession::CleanOutput(std::string& szOutputBuffer, const std::string& szUniqueMarker, size_t markerPos)
{
    size_t lastNonWhitespace;
    std::string szFinalOutput;

    szFinalOutput = szOutputBuffer.substr(0, markerPos);

    lastNonWhitespace = szFinalOutput.find_last_not_of(" \t\n\r");

    if (std::string::npos != lastNonWhitespace) {
        szFinalOutput = szFinalOutput.substr(0, lastNonWhitespace + 1);
    }
    else {
        szFinalOutput.clear();
    }

    return szFinalOutput;
}


std::string PowerShellSession::RunCommand(const std::string& szCommand)
{
    BOOL bIsReadSuccess;
    BOOL bIsWriteSuccess;
    BOOL bIsPipeAvailable;
    CHAR carrReadBuffer[MAX_BUFFER_SIZE];
    DWORD dwBytesRead;
    DWORD dwBytesWritten;
    std::string szOutputBuffer;
    std::string szUniqueMarker = generateUniqueMarker();
    std::string fullCommand = szCommand + GET_CWD +"\nWrite-Host '" + szUniqueMarker + "'\n";

    bIsWriteSuccess = WriteFile(
        hChildStdinWrite,
        fullCommand.c_str(),
        fullCommand.length(),
        &dwBytesWritten,
        NULL
    );

    if (!bIsWriteSuccess) {
        if (GetLastError() == ERROR_NO_DATA) {
            throw std::runtime_error("Failed to write to PowerShell stdin: Pipe is broken. PowerShell might have exited.");
        }
        throw std::runtime_error("Failed to write to PowerShell stdin: " + std::to_string(GetLastError()));
    }

    while (TRUE) {
        bIsPipeAvailable = PeekNamedPipe(hChildStdoutRead, NULL, 0, NULL, &dwBytesRead, NULL);

        if (!bIsPipeAvailable) {
            throw std::runtime_error("Failed to peek PowerShell stdout pipe: " + std::to_string(GetLastError()));
        }
        else if (dwBytesRead > 0) {
            bIsReadSuccess = ReadFile(hChildStdoutRead, carrReadBuffer, sizeof(carrReadBuffer) - 1, &dwBytesRead, NULL);

            if (!bIsReadSuccess || dwBytesRead == 0) {
                throw std::runtime_error("Failed to read from PowerShell stdout: " + std::to_string(GetLastError()));
                break;
            }

            carrReadBuffer[dwBytesRead] = '\0';
            szOutputBuffer.append(carrReadBuffer, dwBytesRead);

            size_t markerPos = szOutputBuffer.find(szUniqueMarker);

            if (markerPos != std::string::npos) {
                return CleanOutput(szOutputBuffer, szUniqueMarker, markerPos);
            }
        }
        else {
            Sleep(10);
        }

    }

    std::cerr << "Warning: Unique marker not found in PowerShell output. Returning partial/full buffer." << std::endl;
    return szOutputBuffer;
}
