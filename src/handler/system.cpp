/*
* CpET 140 Final Project — System Caller module
* StructuraCost - Handler - System Call module
*
* Contributors:
*  Joshua Literal
*
* Purpose
* - Handle system-level operations such as file and directory management, and time retrieval.
*
* Boundaries
* - Interacts with the operating system for file system operations.
*
* Notes
* - This module abstracts away platform-specific details for file and directory handling.
* - Simplifies OS interactions for higher-level application logic.
*/


#include "system.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include "hello_imgui/hello_imgui.h"
// Platform-specific includes for directory creation
#ifdef _WIN32
    #include <direct.h>  // For _mkdir on Windows
    #include <sys/stat.h>  // For _stat on Windows
    #define MKDIR(path) _mkdir(path)
#else
    #include <sys/stat.h>  // For mkdir on Unix/Linux/macOS
    #include <unistd.h>    // For rmdir and other POSIX functions
    #define MKDIR(path) mkdir(path, 0777)
#endif


// Returns the requested part of the current local time.
// Example: fetchTime(PartDateTime::HOUR) -> 0..23
int system::fetchTime(const PartDateTime part) {
    // Get current time
    const auto currentSystemTime = std::chrono::system_clock::now();
    const std::time_t now_c = std::chrono::system_clock::to_time_t(currentSystemTime);
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


// Logs messages in the terminal to the log file.
void system::logMessage(const messageClassification classification, const std::string& message) {
    // Get current time once
    const auto currentSystemTime = std::chrono::system_clock::now();
    const std::time_t now_c = std::chrono::system_clock::to_time_t(currentSystemTime);
    const std::tm* localTime = std::localtime(&now_c);
    if (!localTime) return;

    // Helper for zero-padding
    auto pad2 = [](const int v) { return v < 10 ? "0" + std::to_string(v) : std::to_string(v); };

    // Construct timestamp and filename
    const int year = localTime->tm_year + 1900;
    const int month = localTime->tm_mon + 1;
    const int day = localTime->tm_mday;
    const int hour = localTime->tm_hour;
    const int minute = localTime->tm_min;
    const int second = localTime->tm_sec;

    const std::string timestamp = std::to_string(year) + "-" + pad2(month) + "-" + pad2(day) +
                                  " " + pad2(hour) + ":" + pad2(minute) + ":" + pad2(second);

    const std::string fileName = "logs/log_" + std::to_string(year) + "_" +
                                 std::to_string(month) + "_" + std::to_string(day) + "_" +
                                 std::to_string(hour) + "_" + std::to_string(minute) + "_" +
                                 std::to_string(second) + ".txt";

    // Determine classification prefix
        std::string prefix;
        switch (classification) {
        case messageClassification::INFO:    prefix = "[INFO]"; break;
        case messageClassification::WARNING: prefix = "[WARNING]"; break;
        case messageClassification::ERROR:   prefix = "[ERROR]"; break;
        default:                             prefix = "[INFO]"; break;
        }

    // Write to log file
    if (std::ofstream logFile(fileName, std::ios::app); logFile.is_open()) {
        logFile << "[" << timestamp << "] " << prefix << ": " << message << std::endl;
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


bool system::copyDirectory(const std::string &source, const std::string &destination) {
    namespace fs = std::filesystem;

    try {
        fs::create_directories(destination);
        fs::copy(source, destination,
                 fs::copy_options::recursive |
                 fs::copy_options::overwrite_existing);
        return true;
    }
    catch (const std::exception &e) {
        std::cerr << "Copy failed: " << e.what() << std::endl;
        return false;
    }
}


bool system::deleteDirectory(const std::string& p_directoryName) {
    std::error_code ec;
    const std::filesystem::path dir{p_directoryName};

    // Ensure path exists and is a directory
    if (!std::filesystem::exists(dir, ec) || !std::filesystem::is_directory(dir, ec)) {
        return false;
    }

    // Try noexcept remove_all
    const auto removed = std::filesystem::remove_all(dir, ec);
    if (!ec) {
        return removed > 0;
    }

    // Fallback: attempt platform-specific removal for empty directory
    #ifdef _WIN32
        return _rmdir(p_directoryName.c_str()) == 0;
    #else
        return rmdir(p_directoryName.c_str()) == 0;
    #endif
}


// Create an empty file at the given path, creating parent directories if necessary
bool system::createFile(const std::string& p_filePath) {
    std::error_code errorCode;
    const std::filesystem::path path{p_filePath};

    if (const auto parent = path.parent_path(); !parent.empty() && !std::filesystem::exists(parent, errorCode)) {
        if (!std::filesystem::create_directories(parent, errorCode) || errorCode) {
            return false;
        }
    }

    std::ofstream file(p_filePath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
        file.close();
        return true;
}


bool system::searchFile(const std::string& p_fileName) {
    std::error_code errorCode;
    namespace fs = std::filesystem;
    if (const fs::path p{p_fileName}; fs::exists(p, errorCode)) {
        return fs::is_regular_file(p, errorCode) || fs::is_symlink(p, errorCode) || !fs::is_directory(p, errorCode);
    }
    if (errorCode) {
#ifdef _WIN32
        struct _stat info;
        if (_stat(p_fileName.c_str(), &info) == 0) {
            return (info.st_mode & _S_IFDIR) == 0; // exists and is not a directory
        }
#else
        struct stat info{};
        if (stat(p_fileName.c_str(), &info) == 0) {
            return S_ISREG(info.st_mode) || S_ISLNK(info.st_mode);
        }
#endif
    }
    return false;
}


bool system::deleteFile(const std::string& p_filePath) {
    std::error_code errorCode;
    // Try portable std::filesystem removal (non-throwing)
    if (std::filesystem::remove(p_filePath, errorCode)) {
        return true;
    }
    // If filesystem set an error, try to make file writable and retry
    try {
        std::filesystem::permissions(p_filePath,
                                     std::filesystem::perms::owner_write,
                                     std::filesystem::perm_options::add);
        errorCode.clear();
        if (std::filesystem::remove(p_filePath, errorCode)) {
            return true;
        }
    } catch (...) {
        // ignore and fall through to platform-specific attempts
    }
    // Platform-specific fallbacks
    #ifdef _WIN32
        // Try C std::remove as a fallback; on Windows this maps to _unlink/DeleteFile
        if (std::remove(p_filePath.c_str()) == 0) return true;
    #else
        // POSIX unlink
        if (unlink(p_filePath.c_str()) == 0) return true;
    #endif
    return false;
}


void system::appShutdown() {
    copyDirectory("data", "backup/data_backup" + std::to_string(fetchTime(PartDateTime::YEAR)) + "_" +
                  std::to_string(fetchTime(PartDateTime::MONTH)) + "_" +
                  std::to_string(fetchTime(PartDateTime::DAY)) + "_" +
                  std::to_string(fetchTime(PartDateTime::HOUR)) + "_" +
                  std::to_string(fetchTime(PartDateTime::MINUTE)) + "_" +
                  std::to_string(fetchTime(PartDateTime::SECOND)));
    HelloImGui::GetRunnerParams()->appShallExit = true;
}