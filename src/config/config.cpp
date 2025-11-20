#include "config.h"

// Global authentication flag
bool appConfig::g_auth = false;

// Global test mode flag
bool appConfig::g_testMode = false;

// Definitions of application configuration variables (match declarations in app_config.h)
#include <string>

std::string appConfig::g_appTitle      = "APP TITLE";
std::string appConfig::g_loginTitle    = "LOGIN";
std::string appConfig::g_errorTitle    = "ERROR";

std::string appConfig::g_fontName      = "fonts/OpenSans-Regular.ttf";
std::string appConfig::g_dataDirectory = "data/";
std::string appConfig::g_projectDirectory = "projects/";

std::string appConfig::g_dbNamePayroll = "payroll.db";
std::string appConfig::g_dbNameTracker = "tracker.db";
std::string appConfig::g_dbNameUsers   = "user.db";

int appConfig::g_defaultWidth  = 1600;
int appConfig::g_defaultHeight = 900;

int appConfig::g_loginWidth  = 500;
int appConfig::g_loginHeight = 415;

int appConfig::g_errorWidth  = 300;
int appConfig::g_errorHeight = 100;
