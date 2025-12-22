/*
 * CpET 140 Final Project — Configuration module
 * StructuraCost - Config - Application configuration module
 *
 * Contributors:
 *
 * Purpose
 * - Defines and initializes global application configuration variables (titles, fonts, directories, window sizes).
 * - Matches declarations in config.h and provides default values used across the app.
 *
 * Boundaries
 * - Definitions only; no business logic, I/O, or runtime computation here.
 * - Do not introduce side effects beyond initializing constants/defaults.
 *
 * Notes
 * - Keep this synchronized with config.h declarations.
 * - Values are used by UI and handlers for titles, fonts, paths, and sizes.
 */

#include "config.h"

#include "../handler/system.h"

#include <string>
#include <chrono>


bool appConfig::g_auth = false;
bool appConfig::g_testMode = false;


std::string appConfig::g_appTitle      = "StructuraCost";
std::string appConfig::g_loginTitle    = "Log In Page";
std::string appConfig::g_errorTitle    = "ERROR";

std::string appConfig::g_fontName      = "fonts/OpenSans-Regular.ttf";
std::string appConfig::g_assetsDirectory = "assets/";

std::string appConfig::g_dataDirectory = system::getDocumentDirectory() + "/data/";
std::string appConfig::g_backupDirectory = system::getDocumentDirectory() + "/backup/";
std::string appConfig::g_logsDirectory = system::getDocumentDirectory() + "/logs/";

std::string appConfig::g_projectDirectory = "projects/";
std::string appConfig::g_payrollDirectory  = "payroll/";

std::string appConfig::g_projectExpenseDirectory = "expenses/";

std::string appConfig::g_payrollAttendanceDirectory  = std::to_string(system::fetchTime(system::PartDateTime::YEAR)) + "/";

std::string appConfig::g_dbNamePayroll = "base_payroll.db";
std::string appConfig::g_dbNameProject = "base_project.db";

int appConfig::g_defaultWidth  = 1600;
int appConfig::g_defaultHeight = 900;

int appConfig::g_loginWidth  = 500;
int appConfig::g_loginHeight = 415;

int appConfig::g_errorWidth  = 300;
int appConfig::g_errorHeight = 100;

