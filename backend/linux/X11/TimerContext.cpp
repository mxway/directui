#include "TimerContext.h"
#include <sys/time.h>
#include <atomic>
#include <cassert>

std::atomic_uint32_t timerId { 0 };

TimerContext::TimerContext() :
    m_elements{0}
{

}

TimerContext& TimerContext::GetInstance() {
    static TimerContext timerContext;
    return timerContext;
}

TIMERINFO *TimerContext::AddTimer(UIControl* pSender, UIPaintManager* paintManager, uint32_t intervalValue) {
    std::lock_guard<std::mutex> lockGuard{m_mutex};
    if (m_elements == MAX_TIMEOUT_EVENTS) {
        return nullptr;
    }
    TIMERINFO *timerInfo = new TIMERINFO;
    timerInfo->intervalValue = intervalValue;
    timerInfo->killed = false;
    timerInfo->paintManager = paintManager;
    timerInfo->pSender = pSender;
    timerId = (++timerId)%0xFFFFFFFF;
    timerInfo->uTimerId = timerId;
    this->AddTimer_Internal(timerInfo);
    return timerInfo;
}

uint64_t TimerContext::GetMinimumTimeout() {
    lock_guard<std::mutex>  lockGuard{m_mutex};
    if (m_elements == 0) {
        return 0xFFFFFFFF;
    }
    //取最小堆中的第一个元素，这个就是最快的超时事件。将这个时间做为poll的超时时间。
    //将最快超时的时间（毫秒）减去当前时间，即为poll需要等待的时间。
    auto timerInfo = m_timeout[0];
    struct timeval  now{0};
    gettimeofday(&now, nullptr);
    uint64_t milliseconds = now.tv_sec * 1000 + now.tv_usec/1000;
    return timerInfo->timerOut > milliseconds ? timerInfo->timerOut-milliseconds :0;
}

void TimerContext::ProcessTimeout() {
    lock_guard<std::mutex> lokGuard{m_mutex};
    uint32_t processCount = 0;
    //每次循环最多只处理100个超时事件，以防止程序进入死循环状态。
    struct timeval now {0};
    gettimeofday(&now,nullptr);
    uint64_t milliseconds = now.tv_sec*1000 + now.tv_usec/1000;
    while (m_elements>0 && m_timeout[0]->timerOut<=milliseconds && processCount++<100) {
        auto *timeInfo = m_timeout[0];
        m_timeout[0] = m_timeout[--m_elements];
        ShiftDown(0);
        long lRes = 0;
        timeInfo->paintManager->MessageHandler(DUI_WM_TIMER, (WPARAM)timeInfo->pSender,
                                         (LPARAM)(long)timeInfo->uTimerId, lRes);
        this->AddTimer_Internal(timeInfo);
    }
}

void TimerContext::AddTimer_Internal(TIMERINFO* timerInfo) {
    struct timeval now = {0};
    gettimeofday(&now, nullptr);
    uint64_t milliseconds = now.tv_sec*1000 + now.tv_usec/1000;
    timerInfo->timerOut = milliseconds + timerInfo->intervalValue;
    m_timeout[m_elements] = timerInfo;
    this->ShiftUp(m_elements++);
}

void TimerContext::ShiftUp(int currentIndex) {
    int parentIndex = (currentIndex-1)/2;
    while (currentIndex != 0) {
        assert(parentIndex>=0);
        assert(currentIndex>0);
        if (m_timeout[parentIndex]->timerOut <= m_timeout[currentIndex]->timerOut) {
            break;
        }
        TIMERINFO *tmp = m_timeout[parentIndex];
        m_timeout[parentIndex] = m_timeout[currentIndex];
        m_timeout[currentIndex] = tmp;
        currentIndex = parentIndex;
        parentIndex = (currentIndex-1)/2;
    }
}

void TimerContext::ShiftDown(int currentIndex) {
    int childIndex = currentIndex*2+1;
    while (childIndex<m_elements) {
        //查找右子树的超时时间是否比左子树更小
        if (childIndex+1 < m_elements  && m_timeout[childIndex]->timerOut>m_timeout[childIndex+1]->timerOut) {
            childIndex = childIndex+1;
        }
        if (m_timeout[currentIndex]->timerOut <= m_timeout[childIndex]->timerOut) {
            break;
        }
        TIMERINFO *tmp = m_timeout[currentIndex];
        m_timeout[currentIndex] = m_timeout[childIndex];
        m_timeout[childIndex] = tmp;
        currentIndex = childIndex;
        childIndex = currentIndex*2+1;
    }
}

TIMERINFO *TimerContext::Remove(TIMERINFO* timeInfo) {
    std::lock_guard<std::mutex> lockGuard{m_mutex};
    TIMERINFO *result = nullptr;
    for (int i=0;i<m_elements;i++) {
        if (m_timeout[i] == timeInfo) {
            result = m_timeout[i];
            m_timeout[i] = m_timeout[--m_elements];
            this->ShiftDown(i);
            break;
        }
    }
    return result;
}

TIMERINFO *TimerContext::Remove(uint32_t timerId) {
    std::lock_guard<std::mutex> lockGuard{m_mutex};
    TIMERINFO *result = nullptr;
    for (int i=0;i<m_elements;i++) {
        if (m_timeout[i]->uTimerId == timerId) {
            result = m_timeout[i];
            m_timeout[i] = m_timeout[--m_elements];
            this->ShiftDown(i);
            break;
        }
    }
    return result;
}

void TimerContext::RemoveAllTimers() {
    std::lock_guard<std::mutex> lockGuard{m_mutex};
    m_elements = 0;
}
