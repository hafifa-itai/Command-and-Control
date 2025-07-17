#pragma once

#include "pch.h"


class ThreadSafeQueue {
public:
    VOID Push(std::string szData);

    // Add a version with a timeout for better robustness
    BOOL WaitAndPop(std::string& szResponse, INT iTimeoutMs);

private:
    std::queue<std::string> queue;
    std::mutex mutex;
    std::condition_variable condition;
};
