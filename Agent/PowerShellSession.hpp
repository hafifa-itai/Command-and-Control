#pragma once

#include <windows.h>
#include <iostream>
#include <string>
#include <algorithm>
#define GET_CWD ";$PWD.Path;"

class PowerShellSession
{
public:
    PowerShellSession();
    ~PowerShellSession();

    std::string generateUniqueMarker();
    std::string CleanOutput(std::string& szOutputBuffer, const std::string& szUniqueMarker, size_t markerPos);
    std::string RunCommand(const std::string& szCommandmd);

private:
    HANDLE hChildStdinWrite = NULL;
    HANDLE hChildStdoutRead = NULL;
    PROCESS_INFORMATION piProcInfo = {};
};

