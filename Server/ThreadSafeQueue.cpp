#include "ThreadSafeQueue.hpp"

BOOL ThreadSafeQueue::WaitAndPop(std::wstring& wszOutResponse, INT iTimeoutMs = -1) {
    std::unique_lock<std::mutex> lock(mutex);

    auto wait_condition = [this] { return !queue.empty(); };

    if (iTimeoutMs < 0) {
        condition.wait(lock, wait_condition);
    }
    else {
        if (!condition.wait_for(lock, std::chrono::milliseconds(iTimeoutMs), wait_condition)) {
            return FALSE;
        }
    }

    wszOutResponse = std::move(queue.front());
    queue.pop();
    return TRUE;
}

VOID ThreadSafeQueue::Push(std::wstring wszData)
{
	std::lock_guard<std::mutex> lock(mutex);
    queue.push(std::move(wszData));
    condition.notify_one();
}
