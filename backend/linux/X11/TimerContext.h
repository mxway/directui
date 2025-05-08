#ifndef TIMERCONTEXT_H
#define TIMERCONTEXT_H
#include <UIPaintManager.h>
#include <UIControl.h>
#include <cstdint>
#include <mutex>
#include <map>

typedef struct tagTIMERINFO
{
    UIControl           *pSender;
    uint32_t            uTimerId;
    bool                killed;
    UIPaintManager      *paintManager;
    uint64_t            timerOut;
    uint32_t            intervalValue;
} TIMERINFO;

const uint32_t MAX_TIMEOUT_EVENTS = 512;

class TimerContext {
public:
    TimerContext(const TimerContext&)=delete;
    TimerContext &operator=(const TimerContext&)=delete;
    static TimerContext &GetInstance();
    TIMERINFO           *AddTimer(UIControl *pSender,UIPaintManager *paintManager,uint32_t interval);
    uint64_t            GetMinimumTimeout();
    void                ProcessTimeout();
    TIMERINFO           *Remove(TIMERINFO *timeInfo);
    TIMERINFO           *Remove(uint32_t timerId);
    void                RemoveAllTimers();
private:
    void                AddTimer_Internal(TIMERINFO *timerInfo);
    void                ShiftUp(int currentIndex);
    void                ShiftDown(int currentIndex);
private:
    TimerContext();
private:
    uint32_t                m_elements;
    TIMERINFO               *m_timeout[MAX_TIMEOUT_EVENTS];
    std::mutex      m_mutex;
};



#endif //TIMERCONTEXT_H
