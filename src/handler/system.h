/*
 * CpET 140 Final Project — System utilities module header
 * StructuraCost - Handler - System module
 *
 * Contributors:
 *  Joshua Literal
 *
 * Purpose
 * - Provide system-level utilities such as file operations, logging, and time management
 * - Handle cross-platform file system interactions
 *
 * Boundaries
 * - Used by all modules for common system operations
 *
 * Notes
 * - Includes file deletion, directory creation, logging, and time fetching
 * - Provides platform-specific browser opening functionality
 */

#ifndef CPET_140_FINAL_PROJECT_SYS_H
#define CPET_140_FINAL_PROJECT_SYS_H

#include <string>
#include <vector>


class system {
    public:
        // Which part of the current local date/time to fetch
        enum class PartDateTime {
            YEAR,
            MONTH,
            DAY,
            HOUR,
            MINUTE,
            SECOND
        };

        enum class messageClassification {
            INFO,
            WARNING,
            ERROR,
            FATAL
        };

        // Returns the requested part of the current local time.
        // Example: fetchTime(PartDateTime::HOUR) -> 0..23
        static int fetchTime(PartDateTime part);


        static std::string timeDateString();

        // Logs a message to the rolling daily log file under `logs/`.
        // Thread-safe. Messages below the current log level are ignored.
        static void logMessage(messageClassification classification, const std::string &message);

        // Configure minimum log level to write (default: INFO).
        static void setLogLevel(messageClassification level);
        static messageClassification getLogLevel();

        // Mirror WARNING/ERROR/FATAL to console (stderr). Default: true.
        static void setLogToConsole(bool enabled);
        static bool getLogToConsole();

        // Capture std::cout and std::cerr and forward them to the logger.
        // When enabled, stdout lines are logged as INFO and stderr lines as ERROR.
        // Console mirroring is temporarily disabled while capture is active to avoid loops.
        static void setCaptureStdStreams(bool enable);
        static bool getCaptureStdStreams();

        // Creates a directory with the specified name.
        // Returns true on success, false on failure.
        static bool createDirectory(const std::string &p_directoryName);

        // Check whether a directory exists at the given path. Returns true if
        // the path exists and is a directory.
        static bool searchDirectory(const std::string& p_directoryName);

        // Copy a directory and its contents from source to destination.
        static bool copyDirectory(const std::string &source, const std::string &destination);

        // Remove a directory and its contents. Returns true on success.
        static bool deleteDirectory(const std::string& p_directoryName);

        // Create an empty file at the given path, creating parent directories if necessary.
        static bool createFile(const std::string& p_filePath);

        // Check whether a file exists at the given path. Returns true if the file exists.
        static bool searchFile(const std::string &p_fileName);

        // Delete the file at the given path.
        static bool deleteFile(const std::string& p_filePath);

        // Print payslips for the specified employee IDs from the database.
        // Returns true on success, false on failure.
        static bool printPayslips(const std::string& dbPath, const std::string& logoPath, const std::vector<int>& employeeIds);

        // Print project report for the specified project ID.
        // Returns true on success, false on failure.
        static bool printProjectReport(const std::string& projectId, const std::string& logoPath);

        // Open a file in the system's default browser.
        // Returns true on success, false on failure.
        static bool openFileInBrowser(const std::string& filePath);

        // Application shutdown routine to clean up resources.
        static void appShutdown();

        // ============================================================================
        // Input Validation System
        // ============================================================================

        // Validation types for validateInput function
        enum class ValidationType {
            NOT_EMPTY,              // Check if input is not empty or whitespace
            DIGITS_ONLY,            // Check if input contains only digits
            POSITIVE_INTEGER,       // Check if input is a positive integer (> 0)
            NON_NEGATIVE_INTEGER,   // Check if input is a non-negative integer (>= 0)
            POSITIVE_DECIMAL,       // Check if input is a positive decimal number
            NON_NEGATIVE_DECIMAL,   // Check if input is a non-negative decimal number
            ALPHANUMERIC_SPACES,    // Check if input contains only alphanumeric and spaces
            PROJECT_ID_FORMAT,      // Check if input matches PRJ-##### format
            DATE_FORMAT,            // Check if input matches ISO 8601 date format (YYYY-MM-DD)
            EMPLOYEE_ID,            // Validate employee ID (positive integer)
            NAME,                   // Validate name (letters and spaces only, max 100 chars)
            POSITION,               // Validate position (letters and spaces only, max 50 chars)
            SALARY,                 // Validate salary (numbers and decimal point only, must be > 0)
            HOURS,                  // Validate hours (numbers and decimal point only, 0-168)
            ADVANCE,                // Validate advance (numbers and decimal point only, >= 0, can be empty)
            QUANTITY,               // Validate quantity (non-negative decimal)
            MATERIAL_ID             // Validate material ID (not empty)
        };

        // Validate input based on validation type
        // Returns the input string if validation passes, empty string if it fails
        // validationType: type of validation to perform
        // input: the input string to validate
        static std::string validateInput(ValidationType validationType, const std::string& input);
};


#endif //CPET_140_FINAL_PROJECT_SYS_H
