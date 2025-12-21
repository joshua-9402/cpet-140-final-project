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
#include "print.h"
#include "../config/config.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include "hello_imgui/hello_imgui.h"
#include "../security/cryptography.h"
#include <mutex>
#include <atomic>
#include <iostream>
#include <streambuf>
#include <memory>
// Platform-specific includes only needed for fallback operations
#ifdef _WIN32
    #include <direct.h>
#else
    #include <unistd.h>
#endif


static std::mutex g_logMutex;
static std::atomic<system::messageClassification> g_minLevel{ system::messageClassification::INFO };
static std::atomic<bool> g_logToConsole{ true };

static std::string g_lastLogMessage;
static std::string g_lastLogLine;
static int g_lastLogCount = 0;
static std::streampos g_lastLogPosition = 0;

static std::atomic<bool> g_captureStd{ false };
static bool g_prevConsoleMirror = true;

// Validation helper functions (file-local)
static bool isNotEmpty(const std::string& str) {
    return !str.empty() && std::any_of(str.begin(), str.end(),
        [](unsigned char c) { return !std::isspace(c); });
}

static bool isDigitsOnly(const std::string& str) {
    return !str.empty() && std::all_of(str.begin(), str.end(),
        [](unsigned char c) { return std::isdigit(c); });
}

static bool isLettersAndSpacesOnly(const std::string& str) {
    return !str.empty() && std::all_of(str.begin(), str.end(),
        [](unsigned char c) { return std::isalpha(c) || std::isspace(c); });
}

static bool isValidDecimal(const std::string& str) {
    if (str.empty()) return false;
    bool hasDecimal = false;
    for (char c : str) {
        if (c == '.') {
            if (hasDecimal) return false;
            hasDecimal = true;
        } else if (!std::isdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    return true;
}

static bool isValidProjectID(const std::string& str) {
    if (str.length() < 9 || str.substr(0, 4) != "PRJ-") return false;
    return std::all_of(str.begin() + 4, str.end(),
        [](unsigned char c) { return std::isdigit(c); });
}

static bool isValidISODate(const std::string& str) {
    if (str.length() != 10 || str[4] != '-' || str[7] != '-') return false;

    // Validate digits in correct positions
    for (int i = 0; i < 10; ++i) {
        if (i == 4 || i == 7) continue;
        if (!std::isdigit(static_cast<unsigned char>(str[i]))) return false;
    }

    // Validate date ranges
    try {
        const int year = std::stoi(str.substr(0, 4));
        const int month = std::stoi(str.substr(5, 2));
        const int day = std::stoi(str.substr(8, 2));

        if (year < 1900 || year > 2100 || month < 1 || month > 12) return false;

        const int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        int maxDay = daysInMonth[month - 1];

        if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))) {
            maxDay = 29;
        }

        return day >= 1 && day <= maxDay;
    } catch (...) {
        return false;
    }
}

static bool validatePositiveInteger(const std::string& input) {
    if (!isDigitsOnly(input)) return false;
    try {
        return std::stoll(input) > 0;
    } catch (...) {
        return false;
    }
}

static bool validateNonNegativeInteger(const std::string& input) {
    if (!isDigitsOnly(input)) return false;
    try {
        return std::stoll(input) >= 0;
    } catch (...) {
        return false;
    }
}

static bool validatePositiveDecimal(const std::string& input) {
    if (!isValidDecimal(input)) return false;
    try {
        return std::stod(input) > 0.0;
    } catch (...) {
        return false;
    }
}

static bool validateNonNegativeDecimal(const std::string& input) {
    if (!isValidDecimal(input)) return false;
    try {
        return std::stod(input) >= 0.0;
    } catch (...) {
        return false;
    }
}

// Shutdown helper functions
static void encryptDatabasesOnShutdown() {
    try {
        if (security::DBEncryptionSession::hasKey()) {
            security::DBEncryptionSession::encryptAllDbs();
            security::DBEncryptionSession::clear();
        }
    } catch (const std::exception& e) {
        system::logMessage(system::messageClassification::WARNING,
                         "Error during database encryption at shutdown: " + std::string(e.what()) + "\n");
    }
}

static void cleanOldBackups() {
    try {
        if (!system::searchDirectory(appConfig::g_backupDirectory)) return;

        std::vector<std::pair<std::filesystem::file_time_type, std::filesystem::path>> directories;
        for (const auto& entry : std::filesystem::directory_iterator(appConfig::g_backupDirectory)) {
            if (std::filesystem::is_directory(entry)) {
                directories.emplace_back(std::filesystem::last_write_time(entry), entry.path());
            }
        }

        if (directories.size() <= 20) return;

        std::partial_sort(directories.begin(), directories.begin() + 2, directories.end(),
                        [](const auto &a, const auto &b) { return a.first < b.first; });

        std::filesystem::remove_all(directories[0].second);
        std::filesystem::remove_all(directories[1].second);
    } catch (const std::exception& e) {
        system::logMessage(system::messageClassification::WARNING,
                         "Error cleaning old backups: " + std::string(e.what()) + "\n");
    }
}

static void createBackups() {
    try {
        if (!system::searchDirectory(appConfig::g_backupDirectory)) {
            system::createDirectory(appConfig::g_backupDirectory);
        }
        system::copyDirectory(appConfig::g_dataDirectory, appConfig::g_backupDirectory + "data-" + system::timeDateString());
        system::copyDirectory(appConfig::g_logsDirectory, appConfig::g_backupDirectory + "logs-" + system::timeDateString());
    } catch (const std::exception& e) {
        system::logMessage(system::messageClassification::WARNING,
                         "Error creating backups: " + std::string(e.what()) + "\n");
    }
}

struct LoggerStreamBuf : public std::streambuf {
    std::streambuf* orig;
    system::messageClassification level;
    std::string buffer;
    std::mutex mtx;

    explicit LoggerStreamBuf(std::streambuf* original, system::messageClassification lvl)
        : orig(original), level(lvl) {
        setp(nullptr, nullptr); // we don't use put area buffering
    }

    // Prevent recursion when logger itself writes to std::cerr
    static thread_local bool s_inLogging;

    int overflow(int ch) override {
        if (ch == EOF) return 0;
        const char c = static_cast<char>(ch);
        // Forward to original streambuf
        if (orig) orig->sputc(c);

        // Accumulate and flush on newline
        std::lock_guard<std::mutex> lock(mtx);
        buffer.push_back(c);
        if (c == '\n') {
            if (!s_inLogging) {
                s_inLogging = true;
                // Strip trailing newline for the log line (logger ensures newline)
                std::string line = buffer;
                if (!line.empty() && line.back() == '\n') line.pop_back();
                system::logMessage(level, line);
                s_inLogging = false;
            }
            buffer.clear();
        }
        return ch;
    }

    std::streamsize xsputn(const char* s, std::streamsize n) override {
        if (orig) orig->sputn(s, n);
        std::lock_guard<std::mutex> lock(mtx);
        buffer.append(s, static_cast<size_t>(n));
        // Flush complete lines
        size_t pos = 0;
        while (true) {
            size_t nl = buffer.find('\n', pos);
            if (nl == std::string::npos) break;
            if (!s_inLogging) {
                s_inLogging = true;
                std::string line = buffer.substr(0, nl);
                system::logMessage(level, line);
                s_inLogging = false;
            }
            buffer.erase(0, nl + 1);
            pos = 0;
        }
        return n;
    }

    int sync() override {
        if (orig) orig->pubsync();
        std::lock_guard<std::mutex> lock(mtx);
        if (!buffer.empty()) {
            if (!s_inLogging) {
                s_inLogging = true;
                std::string line = buffer;
                system::logMessage(level, line);
                s_inLogging = false;
            }
            buffer.clear();
        }
        return 0;
    }
};
thread_local bool LoggerStreamBuf::s_inLogging = false;

static std::unique_ptr<LoggerStreamBuf> g_coutBuf;
static std::unique_ptr<LoggerStreamBuf> g_cerrBuf;
static std::streambuf* g_origCout = nullptr;
static std::streambuf* g_origCerr = nullptr;


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


std::string system::timeDateString() {
    return std::to_string(system::fetchTime(PartDateTime::YEAR)) + "_" +
                  std::to_string(system::fetchTime(PartDateTime::MONTH)) + "_" +
                  std::to_string(system::fetchTime(PartDateTime::DAY)) + "_" +
                  std::to_string(system::fetchTime(PartDateTime::HOUR)) + "_" +
                  std::to_string(system::fetchTime(PartDateTime::MINUTE)) + "_" +
                  std::to_string(system::fetchTime(PartDateTime::SECOND));
}

// Get the application support directory path for storing app data
// Returns platform-specific path (e.g., ~/Documents/StructuraCost on macOS/Windows)
std::string system::getAppSupportDirectory() {
    std::string appSupportPath;

#ifdef __APPLE__
    // Prefer the user's Documents folder on macOS
    const char* home = std::getenv("HOME");
    if (home) {
        appSupportPath = std::string(home) + "/Documents/StructuraCost";
    } else {
        // Fallback to Application Support style if HOME not available
        appSupportPath = "./StructuraCost";
    }
#elif defined(_WIN32)
    // Prefer the user's Documents folder on Windows (USERPROFILE + "\\Documents")
    const char* userProfile = std::getenv("USERPROFILE");
    if (userProfile) {
        appSupportPath = std::string(userProfile) + "\\Documents\\StructuraCost";
    } else {
        // Fallback to APPDATA if USERPROFILE not available
        const char* appdata = std::getenv("APPDATA");
        if (appdata) {
            appSupportPath = std::string(appdata) + "\\StructuraCost";
        } else {
            appSupportPath = ".\\StructuraCost"; // last resort
        }
    }
#else
    // Linux/Unix: keep the original behavior (~/.local/share/StructuraCost)
    const char* home = std::getenv("HOME");
    if (home) {
        appSupportPath = std::string(home) + "/.local/share/StructuraCost";
    } else {
        appSupportPath = "./StructuraCost"; // Fallback
    }
#endif

    // Ensure the directory exists
    std::error_code ec;
    if (!std::filesystem::exists(appSupportPath, ec)) {
        std::filesystem::create_directories(appSupportPath, ec);
    }

    return appSupportPath;
}


// Logs messages to a rolling daily log file under logs/ (thread-safe)
void system::logMessage(const messageClassification classification, const std::string& message) {
    // Level filter first (cheap)
    if (static_cast<int>(classification) < static_cast<int>(g_minLevel.load())) {
        return;
    }

    // Get current time once
    const auto currentSystemTime = std::chrono::system_clock::now();
    const std::time_t now_c = std::chrono::system_clock::to_time_t(currentSystemTime);
    const std::tm* localTime = std::localtime(&now_c);
    if (!localTime) return;

    // Helper for zero-padding
    auto pad2 = [](const int v) { return v < 10 ? "0" + std::to_string(v) : std::to_string(v); };

    // Construct timestamp and daily filename
    const int year = localTime->tm_year + 1900;
    const int month = localTime->tm_mon + 1;
    const int day = localTime->tm_mday;
    const int hour = localTime->tm_hour;
    const int minute = localTime->tm_min;
    const int second = localTime->tm_sec;

    const std::string timestamp = std::to_string(year) + "-" + pad2(month) + "-" + pad2(day) +
                                  " " + pad2(hour) + ":" + pad2(minute) + ":" + pad2(second);

    const std::string fileName = appConfig::g_logsDirectory + "structuracost-" + std::to_string(year) + "-" + pad2(month) + "-" + pad2(day) + ".log";

    // Determine classification prefix
    std::string prefix;
    switch (classification) {
        case messageClassification::INFO:    prefix = "[INFO]"; break;
        case messageClassification::WARNING: prefix = "[WARNING]"; break;
        case messageClassification::ERROR:   prefix = "[ERROR]"; break;
        case messageClassification::FATAL:   prefix = "[FATAL]"; break;
        default:                             prefix = "[INFO]"; break;
    }

    const std::string line = "[" + timestamp + "] " + prefix + ": " + message;

    std::lock_guard<std::mutex> lock(g_logMutex);
    // Ensure logs directory exists
    static_cast<void>(createDirectory("logs"));

    // Check if this is a duplicate of the last message
    if (message == g_lastLogMessage && !message.empty()) {
        g_lastLogCount++;

        // Update the last line in the log file with the count
        if (std::fstream logFile(fileName, std::ios::in | std::ios::out); logFile.is_open()) {
            // Seek to the last log position
            logFile.seekp(0, std::ios::end);
            const std::streampos fileSize = logFile.tellp();

            // Only update if we have a valid last position
            if (g_lastLogPosition > 0 && g_lastLogPosition < fileSize) {
                // Go back and overwrite the last line
                logFile.seekp(g_lastLogPosition);
                const std::string updatedLine = "[" + timestamp + "] " + prefix + ": " + message + " (x" + std::to_string(g_lastLogCount + 1) + ")";
                logFile << updatedLine;
                // Pad with spaces to overwrite any remaining characters from the old line
                const std::streamsize oldLength = g_lastLogLine.length();
                const std::streamsize newLength = static_cast<std::streamsize>(updatedLine.length());
                for (std::streamsize i = newLength; i < oldLength; ++i) {
                    logFile << ' ';
                }
                if (!message.empty() && message.back() != '\n') logFile << '\n';
                logFile.flush();
                g_lastLogLine = updatedLine;
            }
        }
    } else {
        // New unique message
        if (std::ofstream logFile(fileName, std::ios::app); logFile.is_open()) {
            g_lastLogPosition = logFile.tellp();
            logFile << line;
            if (!message.empty() && message.back() != '\n') logFile << '\n';
            logFile.flush();
        }

        g_lastLogMessage = message;
        g_lastLogLine = line;
        g_lastLogCount = 0;
    }

    // Optional console mirroring for warnings and above
    if (g_logToConsole.load() && static_cast<int>(classification) >= static_cast<int>(messageClassification::WARNING)) {
        if (message == g_lastLogMessage && g_lastLogCount > 0) {
            // For console, just show the count
            std::cerr << "\r" << line << " (x" << (g_lastLogCount + 1) << ")";
        } else {
            std::cerr << line;
        }
        if (!message.empty() && message.back() != '\n') std::cerr << '\n';
    }
}

void system::setLogLevel(const messageClassification level) {
    g_minLevel.store(level);
}

system::messageClassification system::getLogLevel() {
    return g_minLevel.load();
}

void system::setLogToConsole(const bool enabled) {
    g_logToConsole.store(enabled);
}

bool system::getLogToConsole() {
    return g_logToConsole.load();
}


void system::setCaptureStdStreams(const bool enable) {
    const bool currently = g_captureStd.load();
    if (enable == currently) return;

    if (enable) {
        g_captureStd.store(true);
        // Save originals
        g_origCout = std::cout.rdbuf();
        g_origCerr = std::cerr.rdbuf();
        // Disable console mirroring to avoid echo loops
        g_prevConsoleMirror = g_logToConsole.load();
        g_logToConsole.store(false);
        // Install capturing buffers
        g_coutBuf = std::make_unique<LoggerStreamBuf>(g_origCout, system::messageClassification::INFO);
        g_cerrBuf = std::make_unique<LoggerStreamBuf>(g_origCerr, system::messageClassification::ERROR);
        std::cout.rdbuf(g_coutBuf.get());
        std::cerr.rdbuf(g_cerrBuf.get());
    } else {
        // Restore
        if (g_origCout) std::cout.rdbuf(g_origCout);
        if (g_origCerr) std::cerr.rdbuf(g_origCerr);
        g_coutBuf.reset();
        g_cerrBuf.reset();
        g_origCout = nullptr;
        g_origCerr = nullptr;
        // Restore console mirroring
        g_logToConsole.store(g_prevConsoleMirror);
        g_captureStd.store(false);
    }
}

bool system::getCaptureStdStreams() {
    return g_captureStd.load();
}


bool system::createDirectory(const std::string &p_directoryName) {
    std::error_code ec;

    // Check if already exists
    if (std::filesystem::exists(p_directoryName, ec) &&
        std::filesystem::is_directory(p_directoryName, ec)) {
        return true;
    }

    // Create directories (creates parents as needed)
    return std::filesystem::create_directories(p_directoryName, ec) && !ec;
}


bool system::searchDirectory(const std::string& p_directoryName) {
    std::error_code ec;
    return std::filesystem::is_directory(p_directoryName, ec) && !ec;
}


bool system::copyDirectory(const std::string &source, const std::string &destination) {
    std::error_code ec;

    std::filesystem::create_directories(destination, ec);
    if (ec) {
        logMessage(messageClassification::ERROR, "Failed to create destination directory: " + ec.message() + "\n");
        return false;
    }

    std::filesystem::copy(source, destination,
                          std::filesystem::copy_options::recursive |
                          std::filesystem::copy_options::overwrite_existing, ec);

    if (ec) {
        logMessage(messageClassification::ERROR, "Copy failed: " + ec.message() + "\n");
        return false;
    }

    return true;
}


bool system::deleteDirectory(const std::string& p_directoryName) {
    std::error_code ec;

    if (!std::filesystem::exists(p_directoryName, ec) ||
        !std::filesystem::is_directory(p_directoryName, ec)) {
        return false;
    }

    return std::filesystem::remove_all(p_directoryName, ec) > 0 && !ec;
}


bool system::createFile(const std::string& p_filePath) {
    std::error_code ec;
    const std::filesystem::path path{p_filePath};
    const auto parent = path.parent_path();

    // Create parent directories if needed
    if (!parent.empty() && !std::filesystem::exists(parent, ec)) {
        if (!std::filesystem::create_directories(parent, ec) || ec) {
            return false;
        }
    }

    // Create the file
    std::ofstream file(p_filePath, std::ios::binary);
    return file.is_open();
}


bool system::searchFile(const std::string& p_fileName) {
    std::error_code ec;

    if (!std::filesystem::exists(p_fileName, ec)) {
        return false;
    }

    return std::filesystem::is_regular_file(p_fileName, ec) ||
           std::filesystem::is_symlink(p_fileName, ec);
}


bool system::deleteFile(const std::string& p_filePath) {
    std::error_code ec;

    // Try to remove the file
    if (std::filesystem::remove(p_filePath, ec) && !ec) {
        return true;
    }

    // If failed, try to make file writable and retry
    std::filesystem::permissions(p_filePath,
                                 std::filesystem::perms::owner_write,
                                 std::filesystem::perm_options::add, ec);

    return std::filesystem::remove(p_filePath, ec) && !ec;
}

bool system::printPayslips(const std::string& dbPath, const std::string& logoPath, const std::vector<int>& employeeIds) {
    // The print module will fetch all employees from the database
    (void)dbPath; // dbPath is used by the print module internally
    (void)employeeIds; // Future enhancement: filter by specific IDs

    // Generate output path in data directory
    const std::string outputPath = appConfig::g_dataDirectory + "payslips_output.html";

    // Call the print module to export payslips
    return exportPayslipsHtml(outputPath, logoPath);
}


bool system::printProjectReport(const std::string& projectId, const std::string& logoPath) {
    if (projectId.empty()) {
        logMessage(messageClassification::ERROR, "printProjectReport: project ID is empty\n");
        return false;
    }

    // Generate output path in data directory
    const std::string outputPath = appConfig::g_dataDirectory + "project_report_" + projectId + ".html";

    // Call the print module to export project report
    return exportProjectReportHtml(projectId, outputPath, logoPath);
}

bool system::openFileInBrowser(const std::string& filePath) {
    if (filePath.empty() || !searchFile(filePath)) {
        logMessage(messageClassification::ERROR, "File not found: " + filePath + "\n");
        return false;
    }

    // Convert to absolute path
    std::filesystem::path absPath = std::filesystem::absolute(filePath);
    std::string pathStr = absPath.string();

#ifdef _WIN32
    // Windows: use start command
    std::string command = "start \"\" \"" + pathStr + "\"";
#elif defined(__APPLE__)
    // macOS: use open command
    std::string command = "open \"" + pathStr + "\"";
#else
    // Other platforms are not supported in this build configuration
    logMessage(messageClassification::ERROR, "Opening files in browser is not supported on this platform.\n");
    return false;
#endif

    int result = std::system(command.c_str());
    if (result != 0) {
        logMessage(messageClassification::ERROR, "Failed to open file in browser: " + pathStr + "\n");
        return false;
    }

    logMessage(messageClassification::INFO, "Opened file in browser: " + pathStr + "\n");
    return true;
}

void system::appShutdown() {
    try {
        logMessage(messageClassification::INFO, "Application shutdown initiated.\n");

        encryptDatabasesOnShutdown();
        cleanOldBackups();
        createBackups();

        logMessage(messageClassification::INFO, "Application shutdown completed successfully.\n");
    } catch (const std::exception& e) {
        logMessage(messageClassification::ERROR, "Critical error during shutdown: " + std::string(e.what()) + "\n");
    } catch (...) {
        logMessage(messageClassification::ERROR, "Unknown critical error during shutdown.\n");
    }
}

// ============================================================================
// Input Validation System Implementation
// ============================================================================

std::string system::validateInput(const ValidationType validationType, const std::string& input) {
    switch (validationType) {
        case ValidationType::NOT_EMPTY:
            return isNotEmpty(input) ? input : "";

        case ValidationType::DIGITS_ONLY:
            return isDigitsOnly(input) ? input : "";

        case ValidationType::POSITIVE_INTEGER:
        case ValidationType::EMPLOYEE_ID:
            return validatePositiveInteger(input) ? input : "";

        case ValidationType::NON_NEGATIVE_INTEGER:
            return validateNonNegativeInteger(input) ? input : "";

        case ValidationType::POSITIVE_DECIMAL:
        case ValidationType::SALARY:
            return validatePositiveDecimal(input) ? input : "";

        case ValidationType::NON_NEGATIVE_DECIMAL:
        case ValidationType::QUANTITY:
            return validateNonNegativeDecimal(input) ? input : "";

        case ValidationType::ALPHANUMERIC_SPACES:
            if (!isNotEmpty(input)) return "";
            return std::all_of(input.begin(), input.end(), [](unsigned char c) {
                return std::isalnum(c) || std::isspace(c);
            }) ? input : "";

        case ValidationType::PROJECT_ID_FORMAT:
            return isValidProjectID(input) ? input : "";

        case ValidationType::DATE_FORMAT:
            return isValidISODate(input) ? input : "";

        case ValidationType::NAME:
            return (isNotEmpty(input) && input.length() <= 100 &&
                    isLettersAndSpacesOnly(input)) ? input : "";

        case ValidationType::POSITION:
            return (isNotEmpty(input) && input.length() <= 50 &&
                    isLettersAndSpacesOnly(input)) ? input : "";

        case ValidationType::HOURS:
            if (!isValidDecimal(input)) return "";
            try {
                const double val = std::stod(input);
                return (val >= 0.0 && val <= 168.0) ? input : "";
            } catch (...) {
                return "";
            }

        case ValidationType::ADVANCE:
            if (input.empty() || !isNotEmpty(input)) return input;
            return validateNonNegativeDecimal(input) ? input : "";

        case ValidationType::MATERIAL_ID:
            return isNotEmpty(input) ? input : "";

        case ValidationType::SITE_LOCATION:
            if (!isNotEmpty(input)) return "";
            if (input == "Main Office" || input == "Warehouse") return input;
            return isValidProjectID(input) ? input : "";

        default:
            logMessage(messageClassification::WARNING, "Unknown validation type requested\n");
            return "";
    }
}





