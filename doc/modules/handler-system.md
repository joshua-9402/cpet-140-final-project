# System utilities module (`handler/system.h` / `handler/system.cpp`)

## Purpose

Provides cross-platform system and filesystem helpers used by the application. The module centralizes time utilities, logging, directory/file creation and deletion, copying directories, and an application shutdown routine that performs backup rotation and triggers app exit.

## Files

- `src/handler/system.h`
- `src/handler/system.cpp`

## Public API (summary)

- `enum class system::PartDateTime { YEAR, MONTH, DAY, HOUR, MINUTE, SECOND }` — used with `fetchTime`
- `enum class system::messageClassification { INFO, WARNING, ERROR, FATAL }` — classification for `logMessage`

- `static int system::fetchTime(system::PartDateTime part)`
  - Returns the requested part of the current local time (e.g., 0..23 for HOUR). Returns `-1` on error.

- `static std::string system::timeDateString()`
  - Returns a formatted concatenation of YEAR_MONTH_DAY_HOUR_MINUTE_SECOND using `fetchTime` values.

- `static void system::logMessage(system::messageClassification classification, const std::string &message)`
  - Writes a timestamped log file (under `logs/`) with a filename that includes date/time. Each log file contains a single message line with a classification prefix (`[INFO]`, `[WARNING]`, `[ERROR]`).

- `static bool system::createDirectory(const std::string &p_directoryName)`
  - Creates the directory and parents as needed. Returns `true` if the directory exists after the call (i.e., already existed or was created successfully).
  - Uses `std::filesystem::create_directories` and falls back to platform `mkdir` if necessary.

- `static bool system::searchDirectory(const std::string &p_directoryName)`
  - Returns `true` if the given path exists and is a directory.
  - Uses `std::filesystem::is_directory` and falls back to `stat`/_stat on platforms where exceptions occur.

- `static bool system::copyDirectory(const std::string &source, const std::string &destination)`
  - Copies source directory recursively into destination, creating destination if necessary. Returns `true` on success.

- `static bool system::deleteDirectory(const std::string &p_directoryName)`
  - Removes a directory and its contents. Uses `std::filesystem::remove_all` and falls back to platform-specific removal for simple cases. Returns `true` on success.

- `static bool system::createFile(const std::string &p_filePath)`
  - Creates parent directories if needed and creates an empty file at `p_filePath`. Returns `true` on success.

- `static bool system::searchFile(const std::string &p_fileName)`
  - Returns `true` if a regular file or symlink exists at the path. Uses `std::filesystem` checks and falls back to `stat`/_stat if required.

- `static bool system::deleteFile(const std::string &p_filePath)`
  - Attempts to delete a file using `std::filesystem::remove`. If that fails, attempts to set owner-write permissions then retry; final fallbacks call `unlink`/`remove` on POSIX/Windows.

- `static void system::appShutdown()`
  - Performs application shutdown housekeeping:
    - Scans `backup/` directory for subdirectories and sorts by last-write-time.
    - If more than 20 backup directories exist, removes the two oldest.
    - Copies current `data` and `logs` into new backup directories named with the current timestamp (via `timeDateString`).
    - Signals HelloImGui runner to exit (sets `appShallExit = true`).

## Data shapes

- Time-related functions return integers (YEAR, MONTH, DAY, HOUR, MINUTE, SECOND) or formatted strings.
- File/directory functions accept `std::string` paths and return `bool` for success/failure.
- Log messages are written to plain text files under `logs/`.

## Error modes and return conventions

- IO functions return `bool` indicating success; callers should inspect return values and optionally call `logMessage` on failure.
- `fetchTime` returns `-1` on error; callers must validate the returned integer before use.
- The module uses `std::error_code` to avoid throwing exceptions for common filesystem operations.

## Security considerations

- When creating directories and files that will hold sensitive data, ensure proper permissions are applied after creation (the module ensures creation but callers should set restrictive perms if needed).
- Be careful when copying or removing directories; `deleteDirectory` will recursively remove contents.
- Log files can contain sensitive information — control access to the `logs/` and `backup/` directories appropriately.

## Build / runtime dependencies

- Uses the C++ standard library (`<filesystem>`, `<chrono>`, `<fstream>`). No extra external libraries are required for these utilities.
- On Windows, platform-specific code uses `_stat`/_mkdir; on POSIX systems it uses `stat`, `mkdir`, `rmdir`, and `unlink` as fallbacks.

## Usage examples

```cpp
// Get current hour
int hour = system::fetchTime(system::PartDateTime::HOUR);

// Create data directory
if (!system::createDirectory("data")) {
    system::logMessage(system::messageClassification::ERROR, "Failed to create data directory");
}

// Check if backup file exists
if (system::searchFile("backup/data-2025_11_29/base_payroll.db")) {
    // found
}

// Create an empty marker file (creates parents if needed)
system::createFile("data/.initialized");

// Perform a graceful shutdown (will rotate backups and trigger app exit)
system::appShutdown();
```

## Troubleshooting & debugging tips

- If `searchDirectory` or `searchFile` returns false unexpectedly:
  - Check for platform-specific path differences (absolute vs relative paths).
  - Look for permission issues; ensure the process has read access to the path.
  - If `std::filesystem` throws or sets an `std::error_code`, the implementation falls back to `stat`/_stat — check system logs for details.

- If `createDirectory` fails:
  - Check parent directory permissions.
  - On Windows, path length issues or backslash vs forward-slash are common pitfalls.

- If `deleteFile` fails to remove a file, the function attempts to add owner-write permissions and retry. If that fails, check for other processes locking the file (on Windows) or elevated permissions.

- If `appShutdown` crashes or fails to rotate backups:
  - Ensure the `backup/` directory exists and is readable.
  - If there are many entries in `backup/`, the function uses `std::partial_sort` to find the two oldest. If the standard library on the build environment lacks modern ranges utilities the code uses the portable `std::partial_sort` call.

---

## Repository sync note

Updated to match the exact behavior and API in `src/handler/system.h` and `src/handler/system.cpp` (sync date: 2025-11-29). If you change function names or signatures in the source, update this document accordingly.
