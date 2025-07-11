#include "PowerShellSession.hpp"
#include <stdexcept>
#include <vector>    // For std::vector (used for char buffer in RunCommand)
#include <random>    // For random number generation in generateUniqueMarker
#include <chrono>    // For high_resolution_clock in generateUniqueMarker
#include <algorithm> // For std::find_last_not_of

// Define a macro for safer handle closing
#define CLOSE_HANDLE(handle) \
    if (handle && handle != INVALID_HANDLE_VALUE) { \
        CloseHandle(handle); \
        handle = NULL; \
    }

PowerShellSession::PowerShellSession()
{
    SECURITY_ATTRIBUTES saAttr{};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    HANDLE hChildStdoutReadTmp, hChildStdoutWrite;
    HANDLE hChildStdinRead, hChildStdinWriteTmp;

    // Create pipes for stdout
    CreatePipe(&hChildStdoutReadTmp, &hChildStdoutWrite, &saAttr, 0);
    SetHandleInformation(hChildStdoutReadTmp, HANDLE_FLAG_INHERIT, 0);

    // Create pipes for stdin
    CreatePipe(&hChildStdinRead, &hChildStdinWriteTmp, &saAttr, 0);
    SetHandleInformation(hChildStdinWriteTmp, HANDLE_FLAG_INHERIT, 0);

    // Set up the process startup info
    STARTUPINFOA siStartInfo{};
    siStartInfo.cb = sizeof(STARTUPINFOA);
    siStartInfo.hStdError = hChildStdoutWrite;
    siStartInfo.hStdOutput = hChildStdoutWrite;
    siStartInfo.hStdInput = hChildStdinRead;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    // Create PowerShell process
    char cmd[] = "powershell.exe -NoLogo -NoExit -Command -";
    if (!CreateProcessA(
        NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL,
        &siStartInfo, &piProcInfo)) {
        std::cerr << "Failed to start PowerShell.\n";
        exit(1);
    }

    // Close unused pipe ends
    CloseHandle(hChildStdoutWrite);
    CloseHandle(hChildStdinRead);

    hChildStdoutRead = hChildStdoutReadTmp;
    hChildStdinWrite = hChildStdinWriteTmp;
}

PowerShellSession::~PowerShellSession()
{
    if (hChildStdoutRead) CloseHandle(hChildStdoutRead);
    if (hChildStdinWrite) CloseHandle(hChildStdinWrite);
    if (piProcInfo.hProcess) {
        TerminateProcess(piProcInfo.hProcess, 0);
        CloseHandle(piProcInfo.hProcess);
    }
    if (piProcInfo.hThread) CloseHandle(piProcInfo.hThread);
}

std::string PowerShellSession::generateUniqueMarker() {
    static std::random_device rd; // Seed for random number generator
    static std::mt19937_64 gen(rd()); // Mersenne Twister engine
    std::uniform_int_distribution<unsigned long long> distrib; // Distribution for unsigned long long

    // Combine a random number with a high-resolution timestamp for uniqueness
    return "END_OF_CMD_MARKER_" + std::to_string(distrib(gen)) + "_" +
        std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count());
}


std::string PowerShellSession::RunCommand(const std::string& cmd)
{
    // Basic validation of pipe handles
    if (!hChildStdinWrite || !hChildStdoutRead || hChildStdinWrite == INVALID_HANDLE_VALUE || hChildStdoutRead == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("PowerShell session is not initialized or pipe handles are invalid.");
    }

    // Generate a unique marker to signify the end of the command's output
    std::string uniqueMarker = generateUniqueMarker();

    // Construct the full command to send to PowerShell's stdin:
    // 1. The actual command (`cmd`)
    // 2. A newline character (`\n`) to execute the command
    // 3. A PowerShell command to print our unique marker to stdout (`Write-Host`)
    // 4. Another newline character (`\n`) to execute the marker command
    std::string fullCommand = cmd + "\nWrite-Host '" + uniqueMarker + "'\n";

    DWORD dwWritten;
    // Write the full command string to PowerShell's standard input pipe
    if (!WriteFile(hChildStdinWrite, fullCommand.c_str(), fullCommand.length(), &dwWritten, NULL)) {
        // If WriteFile fails with ERROR_NO_DATA, it means the pipe has been closed,
        // likely because PowerShell exited unexpectedly.
        if (GetLastError() == ERROR_NO_DATA) {
            throw std::runtime_error("Failed to write to PowerShell stdin: Pipe is broken. PowerShell might have exited.");
        }
        throw std::runtime_error("Failed to write to PowerShell stdin: " + std::to_string(GetLastError()));
    }

    std::string outputBuffer; // Accumulates all output read from the pipe
    CHAR chBuf[4096];         // Buffer for reading chunks of data
    DWORD dwRead;
    BOOL bSuccess = FALSE;

    // Loop to continuously read from PowerShell's stdout pipe until the unique marker is found
    while (true) {
        DWORD bytesAvailable = 0;
        // PeekNamedPipe checks if data is available in the pipe without blocking.
        if (!PeekNamedPipe(hChildStdoutRead, NULL, 0, NULL, &bytesAvailable, NULL)) {
            // Error peeking pipe. If ERROR_BROKEN_PIPE, PowerShell has likely terminated.
            if (GetLastError() == ERROR_BROKEN_PIPE) {
                throw std::runtime_error("PowerShell stdout pipe broken. PowerShell might have exited unexpectedly.");
            }
            throw std::runtime_error("Failed to peek PowerShell stdout pipe: " + std::to_string(GetLastError()));
        }

        if (bytesAvailable == 0) {
            // No data immediately available. Check if the PowerShell process is still active.
            DWORD exitCode;
            if (GetExitCodeProcess(piProcInfo.hProcess, &exitCode) && exitCode != STILL_ACTIVE) {
                // PowerShell process has exited, and no more data in the pipe.
                // This indicates an unexpected termination.
                std::cerr << "Warning: PowerShell process exited prematurely while waiting for marker. Exit Code: " << exitCode << std::endl;
                break; // Exit the read loop
            }
            // Process is still active, but no data. Wait a short time before retrying.
            Sleep(50); // Sleep for 50 milliseconds to prevent busy-waiting
            continue;
        }

        // Data is available, proceed to read it
        bSuccess = ReadFile(hChildStdoutRead, chBuf, sizeof(chBuf) - 1, &dwRead, NULL);
        if (!bSuccess || dwRead == 0) {
            // Read error or end of pipe (pipe closed by writer).
            if (GetLastError() == ERROR_BROKEN_PIPE || dwRead == 0) {
                std::cerr << "Warning: PowerShell stdout pipe broken or closed unexpectedly." << std::endl;
            }
            else {
                throw std::runtime_error("Failed to read from PowerShell stdout: " + std::to_string(GetLastError()));
            }
            break; // Exit the read loop
        }

        chBuf[dwRead] = '\0'; // Null-terminate the read buffer for string conversion
        outputBuffer.append(chBuf, dwRead); // Append the read data to the accumulated buffer

        // Check if the unique marker is present in the accumulated output.
        size_t markerPos = outputBuffer.find(uniqueMarker);
        if (markerPos != std::string::npos) {
            // Marker found! This signifies the end of the command's output.
            // The actual output is everything before the marker.
            std::string finalOutput = outputBuffer.substr(0, markerPos);

            // Trim trailing whitespace and newline characters from the output.
            // PowerShell often adds extra newlines or spaces.
            size_t lastNonWhitespace = finalOutput.find_last_not_of(" \t\n\r");
            if (std::string::npos != lastNonWhitespace) {
                finalOutput = finalOutput.substr(0, lastNonWhitespace + 1);
            }
            else {
                // If only whitespace, clear it
                finalOutput.clear();
            }

            // Clear the part of the outputBuffer that was just processed (up to and including the marker).
            // This is crucial for a persistent session, as it leaves only the PowerShell prompt
            // (or any subsequent output) for the next command.
            size_t endOfMarkerLine = outputBuffer.find('\n', markerPos);
            if (endOfMarkerLine != std::string::npos) {
                outputBuffer = outputBuffer.substr(endOfMarkerLine + 1);
            }
            else {
                // If marker is at the very end without a newline, clear entire buffer
                outputBuffer.clear();
            }

            return finalOutput; // Return the extracted command output
        }
    }

    // If the loop exits without finding the unique marker (e.g., PowerShell crashed or hung),
    // print a warning and return whatever output was accumulated.
    std::cerr << "Warning: Unique marker not found in PowerShell output. Returning partial/full buffer." << std::endl;
    // Trim any trailing whitespace from the partial output
    size_t lastNonWhitespace = outputBuffer.find_last_not_of(" \t\n\r");
    if (std::string::npos != lastNonWhitespace) {
        outputBuffer = outputBuffer.substr(0, lastNonWhitespace + 1);
    }
    else {
        outputBuffer.clear();
    }
    return outputBuffer;
}
