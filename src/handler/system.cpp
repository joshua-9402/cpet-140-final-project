#include "system.h"
#include <chrono>

enum class partDateTime {
    YEAR,
    MONTH,
    DAY,
    HOUR,
    MINUTE,
    SECOND
};

int fetchTime(const partDateTime p_part) {
    // Get current time
    const auto now = std::chrono::system_clock::now();
    const std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    const std::tm* localTime = std::localtime(&now_c);

    switch (p_part) {
        case partDateTime::YEAR:   return localTime->tm_year + 1900; // tm_year is years since 1900
        case partDateTime::MONTH:  return localTime->tm_mon + 1;     // tm_mon is 0-11
        case partDateTime::DAY:    return localTime->tm_mday;
        case partDateTime::HOUR:   return localTime->tm_hour;
        case partDateTime::MINUTE: return localTime->tm_min;
        case partDateTime::SECOND: return localTime->tm_sec;
        default: return -1;
    }
}