#include "system.h"
#include <chrono>
#include <filesystem>

// Platform-specific includes for directory creation
#ifdef _WIN32
    #include <direct.h>  // For _mkdir on Windows
    #include <sys/stat.h>  // For _stat on Windows
    #define MKDIR(path) _mkdir(path)
#else
    #include <sys/stat.h>  // For mkdir on Unix/Linux/macOS
    #define MKDIR(path) mkdir(path, 0777)
#endif

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


bool system::createDirectory(const std::string& p_directoryName) {
    const char* folderName = p_directoryName.c_str();

    // Use platform-specific mkdir
    if (MKDIR(folderName) == 0) {
        return true;
    }

    // Check if directory already exists (this is okay)
    #ifdef _WIN32
        struct _stat info;
        if (_stat(folderName, &info) == 0 && (info.st_mode & _S_IFDIR)) {
            return true;  // Directory already exists
        }
    #else
        struct stat info{};
        if (stat(folderName, &info) == 0 && S_ISDIR(info.st_mode)) {
            return true;  // Directory already exists
        }
    #endif

    return false;
}

// Check whether a directory exists at the given path
bool system::searchDirectory(const std::string& p_directoryName) {
    try {
        return std::filesystem::is_directory(p_directoryName);
    } catch (...) {
        // fallback to POSIX/stat or Windows _stat
    }

    #ifdef _WIN32
        struct _stat info;
        if (_stat(p_directoryName.c_str(), &info) == 0) {
            return (info.st_mode & _S_IFDIR) != 0;
        }
    #else
        struct stat info{};
        if (stat(p_directoryName.c_str(), &info) == 0) {
            return S_ISDIR(info.st_mode);
        }
    #endif

    return false;
}
