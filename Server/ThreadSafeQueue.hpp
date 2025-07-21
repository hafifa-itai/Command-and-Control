#pragma once

#include "pch.h"

class ThreadSafeQueue {
public:
    VOID Push(std::wstring wszData);
    BOOL WaitAndPop(std::wstring& wszResponse, INT iTimeoutMs);

private:
    std::mutex mutex;
    std::queue<std::wstring> queue;
    std::condition_variable condition;
};
