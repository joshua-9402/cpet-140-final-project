#include "app_config.h"

// Definitions of application configuration variables
constexpr std::string g_appTitle = "APP TITLE";                      // Main App Title
const std::string g_fontName = "fonts/OpenSans-Regular.ttf";         // Default font path
constexpr std::string g_dbNamePayroll = "payroll.db";                // Database file name for payroll
const std::string g_dbNameTracker = "tracker.db";                    // Database file name for tracker
constexpr std::string g_txtNameUser = "user.txt";                    // User data file name


// Default window dimensions
constexpr int g_defaultWidth = 1600;                                 // Window width in pixels
constexpr int g_defaultHeight = 900;                                 // Window height in pixels


// Small window dimensions (e.g., for error dialogs)
constexpr int g_smallUIWidth = 500;                                  // Error UI width in pixels
constexpr int g_smallUIHeight = 200;                                 // Error UI height in pixels
