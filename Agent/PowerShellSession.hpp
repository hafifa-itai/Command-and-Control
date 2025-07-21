#pragma once

#include "pch.h"
#include "Constants.hpp"
#include "StringUtils.hpp"

class PowerShellSession
{
public:
    PowerShellSession();
    ~PowerShellSession();

    std::string RunCommand(const std::string& szCommandmd);
    std::string CleanOutput(std::string& szOutputBuffer, const std::string& szUniqueMarker, size_t markerPos);
    std::string generateUniqueMarker();

private:
    HANDLE hChildStdinWrite = NULL;
    HANDLE hChildStdoutRead = NULL;
    PROCESS_INFORMATION piProcInfo = {};
};