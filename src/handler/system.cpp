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
#include <regex>
#include <iostream>
#include <string>
#include <string_view>
#include <ranges>
#include <array>
#include <cctype>

#include "print.h"
#include "../config/config.h"
// Platform-specific includes only needed for fallback operations
#ifdef _WIN32
    #include <direct.h>
#else
    #include <unistd.h>
#endif


std::array<std::string, 11> g_errorIndicator{
    "",
    " ",
    "ERROR",
    "FAIL",
    "INVALID",
    "N/A",
    "NONE",
    "error",
    "Fail",
    "Invalid",
    "None"
};


std::string system::validateInput(const inputType p_inputType, const std::string& p_input) {
    auto trim = [](std::string_view s) -> std::string_view {
        auto is_space = [](char c){ return std::isspace(static_cast<unsigned char>(c)); };
        auto first = std::ranges::find_if_not(s, is_space);
        auto last  = std::ranges::find_if_not(s | std::views::reverse, is_space);
        return first < last.base() ? s.substr(first - s.begin(), last.base() - first) : std::string_view{};
    };

    std::string_view input = trim(p_input);
    if (input.empty()) return ""; // empty input allowed

    // static regex for performance
    static const std::regex r_name("^[A-Za-z\\s]+$");
    static const std::regex r_int("^[+-]?\\d+$");
    static const std::regex r_float(R"(^[+-]?(\d+(\.\d*)?|\.\d+)([eE][+-]?\d+)?)");
    static const std::regex r_project("^PRJ-\\d{5}$");
    static const std::regex r_material("^MAT-\\d{5}$");
    static const std::regex r_date(R"(^(?:\d{4}-(?:(?:0[1-9]|1[0-2])-(?:0[1-9]|[12]\d|3[01]))))");

    switch (p_inputType) {
        case NAME:          return std::regex_match(input.begin(), input.end(), r_name) ? std::string(input) : "";
        case EMPLOYEE_ID:   return std::regex_match(input.begin(), input.end(), r_int) ? std::string(input) : "";
        case PROJECT_ID:    return std::regex_match(input.begin(), input.end(), r_project) ? std::string(input) : "";
        case POSITION:      return std::regex_match(input.begin(), input.end(), r_name) ? std::string(input) : "";
        case HOURLY_RATE:
        case REGULAR_HOURS:
        case ADVANCE:
        case NUMBER:        return std::regex_match(input.begin(), input.end(), r_float) ? std::string(input) : "";
        case SITE_LOCATION: return (input == "Main Office" || input == "Warehouse" || std::regex_match(input.begin(), input.end(), r_project)) ? std::string(input) : "";
        case DATE:          return std::regex_match(input.begin(), input.end(), r_date) ? std::string(input) : "";
        case MATERIAL_ID:   return std::regex_match(input.begin(), input.end(), r_material) ? std::string(input) : "";
        default:            return std::string(input); // accept anything if unknown type
    }
}


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


std::string system::dateString() {
    return std::to_string(fetchTime(PartDateTime::YEAR)) + "_" +
                  std::to_string(fetchTime(PartDateTime::MONTH)) + "_" +
                  std::to_string(fetchTime(PartDateTime::DAY));
}


std::string system::timeString() {
    return std::to_string(fetchTime(PartDateTime::HOUR)) + "_" +
                  std::to_string(fetchTime(PartDateTime::MINUTE)) + "_" +
                  std::to_string(fetchTime(PartDateTime::SECOND));
}


std::string system::timeDateString() {
    return std::to_string(fetchTime(PartDateTime::YEAR)) + "_" +
                  std::to_string(fetchTime(PartDateTime::MONTH)) + "_" +
                  std::to_string(fetchTime(PartDateTime::DAY)) + "_" +
                  std::to_string(fetchTime(PartDateTime::HOUR)) + "_" +
                  std::to_string(fetchTime(PartDateTime::MINUTE)) + "_" +
                  std::to_string(fetchTime(PartDateTime::SECOND));
}


// Get the application support directory path for storing app data
// Returns platform-specific path (e.g., ~/Documents/StructuraCost on macOS/Windows)
std::string system::getDocumentDirectory() {
    std::string l_documentPath;

    #ifdef __APPLE__
        if (const char* home = std::getenv("HOME")) l_documentPath = std::string(home) + "/Documents/StructuraCost";
    #elif defined(_WIN32)
        if (const char* userProfile = std::getenv("USERPROFILE")) l_documentPath = std::string(userProfile) + "\\Documents\\StructuraCost";
    #endif

    // Ensure the directory exists
    if (std::error_code ec; !std::filesystem::exists(l_documentPath, ec)) {
        std::filesystem::create_directories(l_documentPath, ec);
    }

    return l_documentPath;
}


void system::performBackup(const backupAction p_action) {
    if (p_action == FULL_LATEST_VERSION) {
        std::vector<std::filesystem::path> dirs;

        // Collect history subdirectories
        for (const auto& entry : std::filesystem::directory_iterator(appConfig::g_backupDirectory + "history/")) {
            if (entry.is_directory()) {
                dirs.emplace_back(entry.path());
            }
        }

        // If more than 10 history folders exist, delete the two oldest
        if (dirs.size() > 10) {
            std::sort(dirs.begin(), dirs.end(),
                [](const std::filesystem::path& a, const std::filesystem::path& b) {
                    std::error_code ec1, ec2;
                    const auto l_time1 = std::filesystem::last_write_time(a, ec1);
                    const auto l_time2 = std::filesystem::last_write_time(b, ec2);

                    if (ec1 || ec2) {
                        return a.string() < b.string(); // deterministic fallback
                    }
                    return l_time1 < l_time2; // oldest first
                });

            for (size_t i = 0; i < 2; ++i) {
                if (!deleteDirectory(dirs[i].string())) {
                    logMessage(messageClassification::WARNING,
                               "Failed to delete old backup folder: " + dirs[i].string());
                } else {
                    logMessage(messageClassification::INFO,
                               "Pruned old backup folder: " + dirs[i].string());
                }
            }
        }
    }

    switch (p_action) {
        case FULL_LATEST_VERSION_OVERWRITE:
            copyDirectory(appConfig::g_payrollDirectory, appConfig::g_backupDirectory + "latest/");
            copyDirectory(appConfig::g_projectDirectory, appConfig::g_backupDirectory + "latest/");
            break;
        case FULL_LATEST_VERSION:
            copyDirectory(appConfig::g_payrollDirectory + dateString(), appConfig::g_backupDirectory + "history/");
            copyDirectory(appConfig::g_projectDirectory + dateString(), appConfig::g_backupDirectory + "history/");
        case DELETE_LATEST_VERSION:
            deleteDirectory(appConfig::g_backupDirectory + "latest/");
            break;
        case DELETE_ALL_BACKUP:
            deleteDirectory(appConfig::g_backupDirectory);
            createDirectory(appConfig::g_backupDirectory);
            break;
        default:
            break;
    }
}


// Logs messages to a unique log file per application session
void system::logMessage(const messageClassification p_classification, const std::string& p_message) {
    if (p_message.empty()) return;

    // Generate log filename once per session (static initialization)
    static const std::string l_fileName = []() {
        return appConfig::g_logsDirectory + "structuracost-" + timeDateString() + ".log";
    }();

    // Determine classification prefix
    std::string l_prefix;
    switch (p_classification) {
        case messageClassification::INFO:    l_prefix = "[INFO]"; break;
        case messageClassification::WARNING: l_prefix = "[WARNING]"; break;
        case messageClassification::ERROR:   l_prefix = "[ERROR]"; break;
        case messageClassification::FATAL:   l_prefix = "[FATAL]"; break;
        default:                             l_prefix = "[INFO]"; break;
    }

    const std::string l_line = "[" + timeDateString() + "] " + l_prefix + ": " + p_message;

    // Ensure logs directory exists
    static_cast<void>(createDirectory(appConfig::g_logsDirectory));

    // Write to log file
    if (std::ofstream logFile(l_fileName, std::ios::app); logFile.is_open()) {
        logFile << l_line << '\n';
    }
}



bool system::createDirectory(const std::string &p_directoryName) {
    std::error_code l_errorCode;

    // Check if already exists
    if (std::filesystem::exists(p_directoryName, l_errorCode) &&
        std::filesystem::is_directory(p_directoryName, l_errorCode)) {
        return true;
    }

    // Create directories (creates parents as needed)
    return std::filesystem::create_directories(p_directoryName, l_errorCode) && !l_errorCode;
}


bool system::searchDirectory(const std::string& p_directoryName) {
    std::error_code l_errorCode;
    return std::filesystem::is_directory(p_directoryName, l_errorCode) && !l_errorCode;
}


bool system::copyDirectory(const std::string &source, const std::string &destination) {
    std::error_code l_errorCode;

    std::filesystem::create_directories(destination, l_errorCode);
    if (l_errorCode) {
        logMessage(messageClassification::ERROR, "Failed to create destination directory: " + l_errorCode.message() + "\n");
        return false;
    }

    std::filesystem::copy(source, destination,
                          std::filesystem::copy_options::recursive |
                          std::filesystem::copy_options::overwrite_existing, l_errorCode);

    if (l_errorCode) {
        logMessage(messageClassification::ERROR, "Copy failed: " + l_errorCode.message() + "\n");
        return false;
    }

    return true;
}


bool system::deleteDirectory(const std::string& p_directoryName) {
    std::error_code l_errorCode;

    if (!std::filesystem::exists(p_directoryName, l_errorCode) ||
        !std::filesystem::is_directory(p_directoryName, l_errorCode)) {
        return false;
    }

    return std::filesystem::remove_all(p_directoryName, l_errorCode) > 0 && !l_errorCode;
}


bool system::createFile(const std::string& p_filePath) {
    const std::filesystem::path l_path{p_filePath};
    const auto l_parent = l_path.parent_path();

    // Create parent directories if needed
    if (std::error_code l_errorCode; !l_parent.empty() && !std::filesystem::exists(l_parent, l_errorCode)) {
        if (!std::filesystem::create_directories(l_parent, l_errorCode) || l_errorCode) {
            return false;
        }
    }

    // Create the file
    const std::ofstream l_file(p_filePath, std::ios::binary);
    return l_file.is_open();
}


bool system::searchFile(const std::string& p_fileName) {
    std::error_code l_errorCode;

    if (!std::filesystem::exists(p_fileName, l_errorCode)) {
        return false;
    }

    return std::filesystem::is_regular_file(p_fileName, l_errorCode) ||
           std::filesystem::is_symlink(p_fileName, l_errorCode);
}


bool system::deleteFile(const std::string& p_filePath) {
    std::error_code l_errorCode;

    // Try to remove the file
    if (std::filesystem::remove(p_filePath, l_errorCode) && !l_errorCode) {
        return true;
    }

    // If failed, try to make file writable and retry
    std::filesystem::permissions(p_filePath,
                                 std::filesystem::perms::owner_write,
                                 std::filesystem::perm_options::add, l_errorCode);

    return std::filesystem::remove(p_filePath, l_errorCode) && !l_errorCode;
}

bool system::printPayslips(const std::string& p_dbPath, const std::string& p_logoPath, const std::vector<int>& p_employeeIds) {
    // The print module will fetch all employees from the database
    (void)p_dbPath; // dbPath is used by the print module internally
    (void)p_employeeIds; // Future enhancement: filter by specific IDs

    // Generate output path in data directory
    const std::string l_outputPath = appConfig::g_dataDirectory + "payslips_output.html";

    // Call the print module to export payslips
    return exportPayslipsHtml(l_outputPath, p_logoPath);
}


bool system::printProjectReport(const std::string& p_projectId, const std::string& l_logoPath) {
    if (p_projectId.empty()) {
        logMessage(messageClassification::ERROR, "printProjectReport: project ID is empty\n");
        return false;
    }

    // Generate output path in data directory
    const std::string l_outputPath = appConfig::g_dataDirectory + "project_report_" + p_projectId + ".html";

    // Call the print module to export project report
    return exportProjectReportHtml(p_projectId, l_outputPath, l_logoPath);
}


bool system::openFileInBrowser(const std::string& p_filePath) {
    if (p_filePath.empty() || !searchFile(p_filePath)) {
        logMessage(messageClassification::ERROR, "File not found: " + p_filePath + "\n");
        return false;
    }

    // Convert to absolute path
    const std::filesystem::path l_absPath = std::filesystem::absolute(p_filePath);
    const std::string l_pathStr = l_absPath.string();

#ifdef _WIN32
    // Windows: use start command
    const std::string l_command = "start \"\" \"" + l_pathStr + "\"";
#elif defined(__APPLE__)
    // macOS: use open command
    const std::string l_command = "open \"" + l_pathStr + "\"";
#else
    // Other platforms are not supported in this build configuration
    logMessage(messageClassification::ERROR, "Opening files in browser is not supported on this platform.\n");
    return false;
#endif

    if (int l_result = std::system(l_command.c_str()); l_result != 0) {
        logMessage(messageClassification::ERROR, "Failed to open file in browser: " + l_pathStr + "\n");
        return false;
    }

    logMessage(messageClassification::INFO, "Opened file in browser: " + l_pathStr + "\n");
    return true;
}


void system::appShutdown() {
    logMessage(messageClassification::INFO, "Application shutdown initiated.\n");
    performBackup(FULL_LATEST_VERSION_OVERWRITE);
    performBackup(FULL_LATEST_VERSION);
    logMessage(messageClassification::INFO, "Application shutdown completed successfully.\n");
}