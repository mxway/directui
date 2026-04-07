#ifndef DIRECTUI_UIELAPSEDTIMER_H
#define DIRECTUI_UIELAPSEDTIMER_H
#include <iostream>
#include <chrono>
#include <string>

using namespace std;
using namespace chrono;

class UIElapsedTimer {
public:
    UIElapsedTimer() {
        m_startTime = high_resolution_clock::now();
    }

    UIElapsedTimer(const string &prefixString)
            : m_prefixString(prefixString) {
        m_startTime = high_resolution_clock::now();
    }


    ~UIElapsedTimer() {
        this->PrintElapsed();
    }

    void Restart() {
        m_startTime = high_resolution_clock::now();
    }

    uint64_t ElapsedMilliseconds() const {
        return duration_cast<milliseconds>(m_endTime - m_startTime).count();
    }

    uint64_t ElapsedMicroseconds()const {
        return duration_cast<microseconds>(m_endTime - m_startTime).count();
    }
    void PrintElapsed() {
        if (m_startTime == high_resolution_clock::time_point()) {
            return;
        }
        m_endTime = high_resolution_clock::now();
        if (m_prefixString != "") {
            cout << m_prefixString << ": ";
        }
        if (this->ElapsedMicroseconds() > 1000) {
            cout << "Elapsed time: " << this->ElapsedMilliseconds() << " ms" << endl;
        } else {
            cout << "Elapsed time: " << this->ElapsedMicroseconds() << " us" << endl;
        }
    }
private:
    high_resolution_clock::time_point m_startTime;
    high_resolution_clock::time_point m_endTime;
    string                            m_prefixString;
};

#endif //DIRECTUI_UIELAPSEDTIMER_H
