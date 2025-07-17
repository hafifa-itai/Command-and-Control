#include "ThreadSafeQueue.hpp"


// Add a version with a timeout for better robustness
BOOL ThreadSafeQueue::WaitAndPop(std::string& szOutResponse, INT iTimeoutMs = -1) {
    std::unique_lock<std::mutex> lock(mutex);

    auto wait_condition = [this] { return !queue.empty(); };

    if (iTimeoutMs < 0) {
        // Wait forever
        condition.wait(lock, wait_condition);
    }
    else {
        // Wait for a specific duration
        if (!condition.wait_for(lock, std::chrono::milliseconds(iTimeoutMs), wait_condition)) {
            return FALSE; // Timed out
        }
    }

    szOutResponse = std::move(queue.front());
    queue.pop();
    return TRUE;
}

void ThreadSafeQueue::Push(std::string szData)
{
	std::lock_guard<std::mutex> lock(mutex);
    queue.push(std::move(szData));
    condition.notify_one();
}
