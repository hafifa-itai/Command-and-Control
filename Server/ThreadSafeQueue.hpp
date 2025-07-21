#pragma once

#include "pch.h"

class ThreadSafeQueue {
public:
    VOID Push(std::string szData);
    BOOL WaitAndPop(std::string& szResponse, INT iTimeoutMs);

private:
    std::mutex mutex;
    std::queue<std::string> queue;
    std::condition_variable condition;
};
