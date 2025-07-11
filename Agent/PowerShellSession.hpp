#pragma once

#include <windows.h>
#include <iostream>
#include <string>
#include <algorithm>

class PowerShellSession
{
public:
    PowerShellSession();
    ~PowerShellSession();

    std::string generateUniqueMarker();

    std::string RunCommand(const std::string& cmd);

private:
    HANDLE hChildStdinWrite = NULL;
    HANDLE hChildStdoutRead = NULL;
    PROCESS_INFORMATION piProcInfo = {};
};

