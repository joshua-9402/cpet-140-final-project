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

        // Logs messages in the terminal to the log file.
        static void logMessage(messageClassification classification, const std::string &message);

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

        // Application shutdown routine to clean up resources.
        static void appShutdown();
};


#endif //CPET_140_FINAL_PROJECT_SYS_H