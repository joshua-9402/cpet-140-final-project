#include "system.h"
#include <chrono>
#include <sys/stat.h> // For mkdir()

int system::fetchTime(const PartDateTime part) {
    // Get current time
    const auto now = std::chrono::system_clock::now();
    const std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    const std::tm* localTime = std::localtime(&now_c);
    if (!localTime) return -1;

    switch (part) {
        case PartDateTime::YEAR:   return localTime->tm_year + 1900; // tm_year is years since 1900
        case PartDateTime::MONTH:  return localTime->tm_mon + 1;     // tm_mon is 0-11
        case PartDateTime::DAY:    return localTime->tm_mday;
        case PartDateTime::HOUR:   return localTime->tm_hour;
        case PartDateTime::MINUTE: return localTime->tm_min;
        case PartDateTime::SECOND: return localTime->tm_sec;
        default: return -1;
    }
}


bool createDirectory(const std::string& p_directoryName) {
    const char* folderName = p_directoryName.c_str();

    if (constexpr mode_t permissions = 0777; mkdir(folderName, permissions) == 0) {return true;}

    return false;
}