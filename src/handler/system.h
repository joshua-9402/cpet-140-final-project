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

        enum inputType {
            EMPLOYEE_ID,            // Validate employee ID (positive integer)
            PROJECT_ID,             // Check if input matches PRJ-##### format
            MATERIAL_ID,            // Validate material ID (MAT-#####)

            NAME,                   // Validate name (letters and spaces only, max 100 chars)
            POSITION,               // Validate position (letters and spaces only, max 50 chars)
            HOURLY_RATE,            // Validate salary (numbers and decimal point only, must be > 0)
            REGULAR_HOURS,          // Validate hours (numbers and decimal point only, 0-168)
            ADVANCE,                // Validate advance (numbers and decimal point only, >= 0, can be empty)
            SITE_LOCATION,          // Validate site location (Main Office, Warehouse, or PRJ-XXXXX)
            DATE,                   // Check if input matches ISO 8601 date format (YYYY-MM-DD)

            NUMBER,                 // General number (positive integer or decimal)
        };

        enum backupAction {
            FULL_LATEST_VERSION_OVERWRITE,
            FULL_LATEST_VERSION,
            DELETE_LATEST_VERSION,
            DELETE_ALL_BACKUP
        };

        // Validates input based on the specified type.
        static std::string validateInput(inputType p_inputType, const std::string &p_input);

        // Returns the requested part of the current local time.
        // Example: fetchTime(PartDateTime::HOUR) -> 0..23
        static int fetchTime(PartDateTime part);

        static std::string dateString();

        static std::string timeString();

        // Returns a string representation of the current date and time
        static std::string timeDateString();

        // Get the application support directory path for storing app data
        // Returns platform-specific path (e.g., ~/Library/Application Support/StructuraCost on macOS)
        static std::string getDocumentDirectory();


        static void performBackup(backupAction p_action);

        // Logs a message to a unique log file per application session under `logs/`.
        // Each time the application starts, a new log file is created with a timestamp.
        // Thread-safe. Messages below the current log level are ignored.
        static void logMessage(messageClassification p_classification, const std::string &p_message);


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
        static bool printPayslips(const std::string& p_dbPath, const std::string& p_logoPath, const std::vector<int>& p_employeeIds);

        // Print project report for the specified project ID.
        // Returns true on success, false on failure.
        static bool printProjectReport(const std::string& p_projectId, const std::string& l_logoPath);

        // Open a file in the system's default browser.
        // Returns true on success, false on failure.
        static bool openFileInBrowser(const std::string& p_filePath);

        // Application shutdown routine to clean up resources.
        static void appShutdown();
};


#endif //CPET_140_FINAL_PROJECT_SYS_H
