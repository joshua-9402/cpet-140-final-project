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
#ifdef _WIN32
    #include <direct.h>  // For _mkdir on Windows
    #include <sys/stat.h>  // For _stat on Windows
    #define MKDIR(path) _mkdir(path)
#else
    #include <sys/stat.h>  // For mkdir on Unix/Linux/macOS
    #include <unistd.h>    // For rmdir and other POSIX functions
    #define MKDIR(path) mkdir(path, 0777)
#endif


namespace {
    // Logging controls (thread-safe)
    std::mutex g_logMutex;
    std::atomic<system::messageClassification> g_minLevel{ system::messageClassification::INFO };
    std::atomic<bool> g_logToConsole{ true };

    // Last log message tracking for duplicate detection
    std::string g_lastLogMessage;
    std::string g_lastLogLine;
    int g_lastLogCount = 0;
    std::streampos g_lastLogPosition = 0;

    // Std stream capture state
    std::atomic<bool> g_captureStd{ false };
    bool g_prevConsoleMirror = true;
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

    std::unique_ptr<LoggerStreamBuf> g_coutBuf;
    std::unique_ptr<LoggerStreamBuf> g_cerrBuf;
    std::streambuf* g_origCout = nullptr;
    std::streambuf* g_origCerr = nullptr;
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


std::string system::timeDateString() {
    return std::to_string(system::fetchTime(PartDateTime::YEAR)) + "_" +
                  std::to_string(system::fetchTime(PartDateTime::MONTH)) + "_" +
                  std::to_string(system::fetchTime(PartDateTime::DAY)) + "_" +
                  std::to_string(system::fetchTime(PartDateTime::HOUR)) + "_" +
                  std::to_string(system::fetchTime(PartDateTime::MINUTE)) + "_" +
                  std::to_string(system::fetchTime(PartDateTime::SECOND));
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

    const std::string fileName = "logs/structuracost-" + std::to_string(year) + "-" + pad2(month) + "-" + pad2(day) + ".log";

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
    namespace fs = std::filesystem;
    std::error_code ec;

    // If already exists and is a directory, we're done.
    if (fs::exists(p_directoryName, ec) && fs::is_directory(p_directoryName, ec)) return true;

    // Try to create directories (creates parents as needed)
    if (fs::create_directories(p_directoryName, ec) && !ec) return true;

    // Fallback to platform mkdir if std::filesystem failed for some reason
    if (const char* folderName = p_directoryName.c_str(); MKDIR(folderName) == 0) return true;

    // Final check: maybe directory now exists (race condition)
    if (fs::exists(p_directoryName, ec) && fs::is_directory(p_directoryName, ec)) return true;

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
    // Windows: use ShellExecute or start command
    std::string command = "start \"\" \"" + pathStr + "\"";
#elif __APPLE__
    // macOS: use open command
    std::string command = "open \"" + pathStr + "\"";
#else
    // Linux: use xdg-open
    std::string command = "xdg-open \"" + pathStr + "\"";
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

        // Ensure databases are encrypted at shutdown if a session key exists
        try {
            if (security::DBEncryptionSession::hasKey()) {
                security::DBEncryptionSession::encryptAllDbs();
                security::DBEncryptionSession::clear();
            }
        } catch (const std::exception& e) {
            logMessage(messageClassification::WARNING, "Error during database encryption at shutdown: " + std::string(e.what()) + "\n");
        }

        // Clean up old backups
        try {
            if (searchDirectory("backup")) {
                std::vector<std::pair<std::filesystem::file_time_type, std::filesystem::path>> directories;
                for (const auto& entry : std::filesystem::directory_iterator("backup")) {
                    if (std::filesystem::is_directory(entry)) {
                        directories.emplace_back(std::filesystem::last_write_time(entry), entry.path());
                    }
                }

                if (directories.size() > 20) {
                    std::partial_sort(directories.begin(), directories.begin() + 2, directories.end(),
                                      [](auto const &a, auto const &b) { return a.first < b.first; });

                    // Remove the two oldest backup directories
                    std::filesystem::remove_all(directories[0].second);
                    std::filesystem::remove_all(directories[1].second);
                }
            }
        } catch (const std::exception& e) {
            logMessage(messageClassification::WARNING, "Error cleaning old backups: " + std::string(e.what()) + "\n");
        }

        // Create new backups
        try {
            if (!searchDirectory("backup")) {
                createDirectory("backup");
            }
            copyDirectory("data", "backup/data-" + timeDateString());
            copyDirectory("logs", "backup/logs-" + timeDateString());
        } catch (const std::exception& e) {
            logMessage(messageClassification::WARNING, "Error creating backups: " + std::string(e.what()) + "\n");
        }

        logMessage(messageClassification::INFO, "Application shutdown completed successfully.\n");
    } catch (const std::exception& e) {
        logMessage(messageClassification::ERROR, "Critical error during shutdown: " + std::string(e.what()) + "\n");
    } catch (...) {
        logMessage(messageClassification::ERROR, "Unknown critical error during shutdown.\n");
    }
    // Return normally to allow main() to exit gracefully
}

// ============================================================================
// Input Validation System Implementation
// ============================================================================

std::string system::validateInput(const ValidationType validationType, const std::string& input) {
    // Helper lambda functions for validation
    auto isNotEmpty = [](const std::string& str) -> bool {
        if (str.empty()) return false;
        for (char c : str) {
            if (!std::isspace(static_cast<unsigned char>(c))) return true;
        }
        return false;
    };

    auto isDigitsOnly = [](const std::string& str) -> bool {
        if (str.empty()) return false;
        for (char c : str) {
            if (!std::isdigit(static_cast<unsigned char>(c))) return false;
        }
        return true;
    };

    // Only letters and spaces (for names and positions)
    auto isLettersAndSpacesOnly = [](const std::string& str) -> bool {
        if (str.empty()) return false;
        for (char c : str) {
            if (!std::isalpha(static_cast<unsigned char>(c)) && !std::isspace(static_cast<unsigned char>(c))) {
                return false;
            }
        }
        return true;
    };

    // Only digits and decimal point (for numeric values)
    auto isValidDecimal = [](const std::string& str) -> bool {
        if (str.empty()) return false;
        bool hasDecimal = false;
        for (char c : str) {
            if (c == '.') {
                if (hasDecimal) return false; // Multiple decimal points
                hasDecimal = true;
            } else if (!std::isdigit(static_cast<unsigned char>(c))) {
                return false;
            }
        }
        return true;
    };

    auto isValidProjectID = [](const std::string& str) -> bool {
        if (str.length() < 9) return false; // "PRJ-" + at least 5 digits
        if (str.substr(0, 4) != "PRJ-") return false;
        for (size_t i = 4; i < str.length(); ++i) {
            if (!std::isdigit(static_cast<unsigned char>(str[i]))) return false;
        }
        return true;
    };

    // Strict ISO 8601 date format validation (YYYY-MM-DD)
    auto isValidISODate = [](const std::string& str) -> bool {
        if (str.length() != 10) return false;
        if (str[4] != '-' || str[7] != '-') return false;

        // Validate all digits in correct positions
        for (int i = 0; i < 10; ++i) {
            if (i == 4 || i == 7) continue;
            if (!std::isdigit(static_cast<unsigned char>(str[i]))) return false;
        }

        // Validate date ranges
        try {
            const int year = std::stoi(str.substr(0, 4));
            const int month = std::stoi(str.substr(5, 2));
            const int day = std::stoi(str.substr(8, 2));

            // Year validation
            if (year < 1900 || year > 2100) return false;

            // Month validation
            if (month < 1 || month > 12) return false;

            // Day validation based on month
            const int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
            int maxDay = daysInMonth[month - 1];

            // Leap year check for February
            if (month == 2) {
                const bool isLeapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
                if (isLeapYear) maxDay = 29;
            }

            return day >= 1 && day <= maxDay;
        } catch (...) {
            return false;
        }
    };

    // Perform validation based on type
    switch (validationType) {
        case ValidationType::NOT_EMPTY:
            return isNotEmpty(input) ? input : "";

        case ValidationType::DIGITS_ONLY:
            return isDigitsOnly(input) ? input : "";

        case ValidationType::POSITIVE_INTEGER:
            if (!isDigitsOnly(input)) return "";
            try {
                const long long val = std::stoll(input);
                return val > 0 ? input : "";
            } catch (...) {
                return "";
            }

        case ValidationType::NON_NEGATIVE_INTEGER:
            if (!isDigitsOnly(input)) return "";
            try {
                const long long val = std::stoll(input);
                return val >= 0 ? input : "";
            } catch (...) {
                return "";
            }

        case ValidationType::POSITIVE_DECIMAL:
            if (!isValidDecimal(input)) return "";
            try {
                const double val = std::stod(input);
                return val > 0.0 ? input : "";
            } catch (...) {
                return "";
            }

        case ValidationType::NON_NEGATIVE_DECIMAL:
            if (!isValidDecimal(input)) return "";
            try {
                const double val = std::stod(input);
                return val >= 0.0 ? input : "";
            } catch (...) {
                return "";
            }

        case ValidationType::ALPHANUMERIC_SPACES:
            if (!isNotEmpty(input)) return "";
            for (char c : input) {
                if (!std::isalnum(static_cast<unsigned char>(c)) &&
                    !std::isspace(static_cast<unsigned char>(c))) {
                    return "";
                }
            }
            return input;

        case ValidationType::PROJECT_ID_FORMAT:
            return isValidProjectID(input) ? input : "";

        case ValidationType::DATE_FORMAT:
            return isValidISODate(input) ? input : "";

        case ValidationType::EMPLOYEE_ID:
            if (!isNotEmpty(input)) return "";
            if (!isDigitsOnly(input)) return "";
            try {
                const long long val = std::stoll(input);
                return val > 0 ? input : "";
            } catch (...) {
                return "";
            }

        case ValidationType::NAME:
            if (!isNotEmpty(input)) return "";
            if (input.length() > 100) return "";
            // Name must contain only letters and spaces
            if (!isLettersAndSpacesOnly(input)) return "";
            return input;

        case ValidationType::POSITION:
            if (!isNotEmpty(input)) return "";
            if (input.length() > 50) return "";
            // Position must contain only letters and spaces
            if (!isLettersAndSpacesOnly(input)) return "";
            return input;

        case ValidationType::SALARY:
            if (!isNotEmpty(input)) return "";
            // Salary must be numeric (digits and decimal point only)
            if (!isValidDecimal(input)) return "";
            try {
                const double val = std::stod(input);
                return val > 0.0 ? input : "";
            } catch (...) {
                return "";
            }

        case ValidationType::HOURS:
            if (!isNotEmpty(input)) return "";
            // Hours must be numeric (digits and decimal point only)
            if (!isValidDecimal(input)) return "";
            try {
                const double val = std::stod(input);
                if (val < 0.0 || val > 168.0) return ""; // Max hours in a week
                return input;
            } catch (...) {
                return "";
            }

        case ValidationType::ADVANCE:
            // Empty is allowed for advance (defaults to 0)
            if (input.empty() || !isNotEmpty(input)) return input;
            // Advance must be numeric (digits and decimal point only)
            if (!isValidDecimal(input)) return "";
            try {
                const double val = std::stod(input);
                return val >= 0.0 ? input : "";
            } catch (...) {
                return "";
            }

        case ValidationType::QUANTITY:
            if (!isNotEmpty(input)) return "";
            if (!isValidDecimal(input)) return "";
            try {
                const double val = std::stod(input);
                return val >= 0.0 ? input : "";
            } catch (...) {
                return "";
            }

        case ValidationType::MATERIAL_ID:
            return isNotEmpty(input) ? input : "";

        case ValidationType::SITE_LOCATION:
            if (!isNotEmpty(input)) return "";
            // Accept "Main Office", "Warehouse", or "PRJ-XXXXX" format
            if (input == "Main Office" || input == "Warehouse") {
                return input;
            }
            // Check if it matches PRJ-XXXXX format
            if (isValidProjectID(input)) {
                return input;
            }
            return "";

        default:
            logMessage(messageClassification::WARNING, "Unknown validation type requested\n");
            return "";
    }
}



