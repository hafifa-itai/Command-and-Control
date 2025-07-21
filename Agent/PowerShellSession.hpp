#pragma once

#include "pch.h"
#include "Constants.hpp"
#include <stdexcept>
#include <random>

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

