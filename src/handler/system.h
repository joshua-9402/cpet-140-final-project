#ifndef CPET_140_FINAL_PROJECT_SYS_H
#define CPET_140_FINAL_PROJECT_SYS_H

#include <string>


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

        // Returns the requested part of the current local time.
        // Example: fetchTime(PartDateTime::HOUR) -> 0..23
        static int fetchTime(PartDateTime part);

        // Logs messages in the terminal to the log file.
        static void logMessage();

        // Creates a directory with the specified name.
        // Returns true on success, false on failure.
        static bool createDirectory(const std::string& directoryName);

        // Check whether a directory exists at the given path. Returns true if
        // the path exists and is a directory.
        static bool searchDirectory(const std::string& directoryName);

        // Remove a directory and its contents. Returns true on success.
        static bool deleteDirectory(const std::string& directoryName);

        // Create an empty file at the given path, creating parent directories if necessary.
        static bool createFile(const std::string& p_filePath);

        // Check whether a file exists at the given path.
        static bool searchFile(const std::string& p_filePath);

        // Delete the file at the given path.
        static bool deleteFile(const std::string& p_filePath);
};


#endif //CPET_140_FINAL_PROJECT_SYS_H