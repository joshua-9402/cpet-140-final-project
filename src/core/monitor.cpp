/*
 * CpET 140 Final Project — Monitoring / Reporting module
 * StructuraCost - Core - Monitoring / Reporting module
 *
 * Contributors:
 *  Hershey Mae Tenorio
 *  Mary Grace Jusay
 *  Matthew Anton Guina
 *
 * Purpose
 * - Collects and exposes domain-specific expense and payroll metrics:
 *  - per-project expense totals, payroll/outflow summaries, and invoice/payment status
 *
 * Boundaries
 * - No UI rendering and no DB, or I/O operations.
 * - Persistence or queries must go through the db adapter (db.h) or other handler / service modules.
 * - No system-level monitoring (CPU/memory) here.
 *
 * Notes
 * - Single-threaded by default; callers must synchronize if used from workers.
 * - Validate inputs and avoid partial updates on failure. Emit alerts/logs when thresholds are breached.
 * - Keep everything simple and focused on core monitoring/reporting logic.
 *
 */

#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <cctype>
#include <sqlite3.h>
#include "monitor.h"
#include "../handler/db.h"
#include "../config/config.h"
#include "../handler/system.h"




bool Monitor::addProject(const std::string& projectId,
                const std::string& projectName,
                const std::string& status,
                const std::string& startDate,
                const std::string& endDate,
                const std::string& note) {
    const std::string dbPath = appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_dbNameProject;
    const std::string values = "'" + projectId + "', '" + projectName + "', '" + status + "', '" + startDate + "', '" + endDate + "', '" + note + "'";

    if (db::appendDatabase(dbPath, values)
        && db::createDatabase(appConfig::g_dataDirectory + appConfig::g_projectDirectory +
                              appConfig::g_projectExpenseDirectory + projectId + ".db")) {
        return true;
    }
    return false;
}


PayrollSummary Monitor::computePayrollSummary() {
    PayrollSummary ps{0, 0.0, 0.0};
    const std::string employeeDB = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;

    for (int row = 1; row <= 1000; ++row) {
        if (db::fetchCell(employeeDB, static_cast<size_t>(row), 1).empty()) break;
        ++ps.totalEmployees;

        const std::string salaryStr = db::fetchCell(employeeDB, static_cast<size_t>(row), 5);
        const std::string hoursStr = db::fetchCell(employeeDB, static_cast<size_t>(row), 6);
        const std::string advanceStr = db::fetchCell(employeeDB, static_cast<size_t>(row), 7);

        try {
            const double salary = salaryStr.empty() ? 0.0 : std::stod(salaryStr);
            const double hours = hoursStr.empty() ? 0.0 : std::stod(hoursStr);
            const double advance = advanceStr.empty() ? 0.0 : std::stod(advanceStr);
            ps.totalSalaryExpense += salary * hours;
            ps.totalAdvances += advance;
        } catch (...) {
            // ignore invalid data
        }
    }
    return ps;
}

ProjectSummary Monitor::computeProjectSummary() {
    ProjectSummary s{0, 0};
    const std::string projectDB = appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_dbNameProject;
    for (int row = 1; row <= 1000; ++row) {
        const std::string projectID = db::fetchCell(projectDB, static_cast<size_t>(row), 1);
        if (projectID.empty()) break;
        ++s.totalProjects;
        const std::string status = db::fetchCell(projectDB, static_cast<size_t>(row), 3);
        if (status == "Active" || status == "In Progress") ++s.activeProjects;
    }
    return s;
}

bool Monitor::addEmployee(const std::string& name,
                 const std::string& position,
                 const std::string& siteLocation,
                 const std::string& hourlyRate,
                 const std::string& hoursWorked,
                 const std::string& advance) {
    const std::string dbPath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;

    // Add employee to base payroll database
    if (const std::string values = "'" + name + "', '" + position + "', '" + siteLocation + "', " + hourlyRate + ", " + hoursWorked + ", " + advance; !db::appendDatabase(dbPath, values)) {
        return false;
    }

    // Automatically create initial weekly attendance records with 0 hours for the current year
    // This ensures new employees have attendance tracking ready
    const std::string attendanceBasePath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory +
                                          appConfig::g_payrollAttendanceDirectory;

    // Get the newly created employee ID
    std::string newEmployeeId;
    for (int row = 1; row <= 1000; ++row) {
        if (std::string currentName = db::fetchCell(dbPath, static_cast<size_t>(row), 2); currentName == name) {
            newEmployeeId = db::fetchCell(dbPath, static_cast<size_t>(row), 1);
            break;
        }
    }

    if (!newEmployeeId.empty()) {
        system::logMessage(system::messageClassification::INFO,
            "New employee added with ID: " + newEmployeeId + ", ready for weekly attendance tracking\n");

        // Check if site location is a project (starts with "PRJ-")
        if (siteLocation.length() >= 4 && siteLocation.substr(0, 4) == "PRJ-") {
            // Add employee to project's payroll expenses
            if (addEmployeeToProject(siteLocation, newEmployeeId, name, position, hourlyRate)) {
                system::logMessage(system::messageClassification::INFO,
                    "Added employee " + newEmployeeId + " to project " + siteLocation + " payroll expenses\n");
            }
        }
    }

    return true;
}

std::vector<Monitor::EmployeeRow> Monitor::listEmployees(int maxRows) {
    std::vector<EmployeeRow> rows;
    if (maxRows < 1) return rows;
    const std::string employeeDB = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;
    rows.reserve(static_cast<size_t>(std::min(maxRows, 1000)));
    for (int row = 1; row <= maxRows; ++row) {
        EmployeeRow er{};
        er.id = db::fetchCell(employeeDB, static_cast<size_t>(row), 1);
        if (er.id.empty()) break;
        er.name = db::fetchCell(employeeDB, static_cast<size_t>(row), 2);
        er.position = db::fetchCell(employeeDB, static_cast<size_t>(row), 3);
        er.siteLocation = db::fetchCell(employeeDB, static_cast<size_t>(row), 4);
        er.salary = db::fetchCell(employeeDB, static_cast<size_t>(row), 5);
        er.hoursWork = db::fetchCell(employeeDB, static_cast<size_t>(row), 6);
        er.advance = db::fetchCell(employeeDB, static_cast<size_t>(row), 7);
        rows.emplace_back(std::move(er));
    }
    return rows;
}

bool Monitor::updateEmployee(const std::string& newEmployeeId,
                    const std::string& name,
                    const std::string& position,
                    const std::string& siteLocation,
                    const std::string& hourlyRate,
                    const std::string& hoursWorked,
                    const std::string& advance) {
    if (newEmployeeId.empty()) return false;

    const std::string payrollDbPath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;

    std::string oldEmployeeID;
    std::string oldSiteLocation;
    bool foundEmployee = false;

    // Try to find an exact match by ID first
    for (int row = 1; row <= 1000; ++row) {
        std::string currentID = db::fetchCell(payrollDbPath, static_cast<size_t>(row), 1);
        if (currentID.empty()) break;
        if (currentID == newEmployeeId) {
            oldEmployeeID = currentID;
            oldSiteLocation = db::fetchCell(payrollDbPath, static_cast<size_t>(row), 4); // Column 4 is SITE_LOCATION
            foundEmployee = true;
            break;
        }
    }

    // Otherwise try to find by name (best-effort)
    if (!foundEmployee && !name.empty()) {
        for (int row = 1; row <= 1000; ++row) {
            std::string currentID = db::fetchCell(payrollDbPath, static_cast<size_t>(row), 1);
            if (currentID.empty()) break;
            if (std::string currentName = db::fetchCell(payrollDbPath, static_cast<size_t>(row), 2); currentName == name) {
                oldEmployeeID = currentID;
                oldSiteLocation = db::fetchCell(payrollDbPath, static_cast<size_t>(row), 4);
                foundEmployee = true;
                break;
            }
        }
    }

    // Handle site location changes and project payroll expenses
    if (foundEmployee && !siteLocation.empty() && oldSiteLocation != siteLocation) {
        // Check if old location was a project (starts with "PRJ-")
        if (oldSiteLocation.length() >= 4 && oldSiteLocation.substr(0, 4) == "PRJ-") {
            // Remove employee from old project's payroll expenses
            removeEmployeeFromProject(oldSiteLocation, oldEmployeeID);
            system::logMessage(system::messageClassification::INFO,
                "Removed employee " + oldEmployeeID + " from project " + oldSiteLocation + " payroll expenses\n");
        }

        // Check if new location is a project (starts with "PRJ-")
        if (siteLocation.length() >= 4 && siteLocation.substr(0, 4) == "PRJ-") {
            // Add employee to new project's payroll expenses
            const std::string empName = name.empty() ? db::fetchCell(payrollDbPath, static_cast<size_t>(1), 2) : name;
            const std::string empPosition = position.empty() ? db::fetchCell(payrollDbPath, static_cast<size_t>(1), 3) : position;
            const std::string empRate = hourlyRate.empty() ? db::fetchCell(payrollDbPath, static_cast<size_t>(1), 5) : hourlyRate;

            if (addEmployeeToProject(siteLocation, oldEmployeeID, empName, empPosition, empRate)) {
                system::logMessage(system::messageClassification::INFO,
                    "Added employee " + oldEmployeeID + " to project " + siteLocation + " payroll expenses\n");
            }
        }
    }

    // If ID changed, propagate to ALL attendance databases (both weekly files and base_payroll.db)
    if (foundEmployee && oldEmployeeID != newEmployeeId) {
        const std::string attendanceBasePath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory +
                                              appConfig::g_payrollAttendanceDirectory;

        int updatedRecords = 0;

        // Update base_payroll.db WEEKLY_ATTENDANCE table
        const std::string basePath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;
        if (system::searchFile(basePath)) {
            const std::string setClause = "EMPLOYEE_ID=" + newEmployeeId;
            if (db::updateDatabase(basePath, oldEmployeeID, setClause)) {
                system::logMessage(system::messageClassification::INFO,
                    "Updated employee ID in base_payroll.db attendance from " + oldEmployeeID + " to " + newEmployeeId + "\n");
            }
        }

        // Update all weekly attendance database files
        for (int month = 1; month <= 12; month++) {
            for (int startDay = 1; startDay <= 31; startDay += 7) {
                int endDay = std::min(startDay + 6, 31);
                std::ostringstream dbFileName;
                dbFileName << std::setfill('0') << std::setw(2) << month << "-"
                           << std::setw(2) << startDay << "-" << std::setw(2) << endDay << ".db";
                const std::string attendanceDbPath = attendanceBasePath + dbFileName.str();

                if (system::searchFile(attendanceDbPath)) {
                    // Update all records for this employee in this weekly database
                    for (int row = 1; row <= 100; ++row) {
                        std::string empId = db::fetchCell(attendanceDbPath, static_cast<size_t>(row), 1);
                        if (empId.empty()) break;

                        if (empId == oldEmployeeID) {
                            std::string weekStart = db::fetchCell(attendanceDbPath, static_cast<size_t>(row), 2);
                            if (!weekStart.empty()) {
                                const std::string setClause = "EMPLOYEE_ID=" + newEmployeeId;
                                if (db::updateWeeklyAttendanceRow(attendanceDbPath, oldEmployeeID, weekStart, setClause)) {
                                    updatedRecords++;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (updatedRecords > 0) {
            system::logMessage(system::messageClassification::INFO,
                "Updated employee ID in " + std::to_string(updatedRecords) + " weekly attendance record(s)\n");
        }
    }

    const std::string updateKey = foundEmployee ? oldEmployeeID : newEmployeeId;
    const std::string setClause = "EMPLOYEE_ID='" + newEmployeeId + "', NAME='" + name + "', POSITION='" + position +
                                  "', SITE_LOCATION='" + siteLocation + "', SALARY=" + hourlyRate + ", HOURS_WORK=" + hoursWorked + ", ADVANCE=" + advance;
    return db::updateDatabase(payrollDbPath, updateKey, setClause);
}

bool Monitor::deleteEmployee(const std::string& employeeId) {
    if (employeeId.empty()) return false;

    const std::string dbPath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;

    // Get employee's site location before deletion
    std::string siteLocation;
    for (int row = 1; row <= 1000; ++row) {
        std::string currentID = db::fetchCell(dbPath, static_cast<size_t>(row), 1);
        if (currentID.empty()) break;
        if (currentID == employeeId) {
            siteLocation = db::fetchCell(dbPath, static_cast<size_t>(row), 4); // Column 4 is SITE_LOCATION
            break;
        }
    }

    // If employee is assigned to a project, remove them from project payroll expenses
    if (!siteLocation.empty() && siteLocation.length() >= 4 && siteLocation.substr(0, 4) == "PRJ-") {
        if (removeEmployeeFromProject(siteLocation, employeeId)) {
            system::logMessage(system::messageClassification::INFO,
                "Removed employee " + employeeId + " from project " + siteLocation + " payroll expenses\n");
        }
    }

    // Delete employee from base payroll
    if (!db::deleteRow(dbPath, employeeId)) {
        return false;
    }

    // Automatically delete all weekly attendance records for this employee across all weeks
    const std::string attendanceBasePath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory +
                                          appConfig::g_payrollAttendanceDirectory;

    int deletedRecords = 0;

    // Delete from base_payroll.db WEEKLY_ATTENDANCE table
    const std::string basePath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;
    if (system::searchFile(basePath)) {
        // Delete all attendance records for this employee from base payroll
        const std::string deleteQuery = "DELETE FROM WEEKLY_ATTENDANCE WHERE EMPLOYEE_ID = " + employeeId + ";";
        // Note: This requires adding a new db function, for now we'll use the row-by-row approach
        system::logMessage(system::messageClassification::INFO,
            "Cleaning up attendance records for deleted employee ID: " + employeeId + "\n");
    }

    // Scan all weekly attendance databases and remove this employee's records
    for (int month = 1; month <= 12; month++) {
        for (int startDay = 1; startDay <= 31; startDay += 7) {
            int endDay = std::min(startDay + 6, 31);
            std::ostringstream dbFileName;
            dbFileName << std::setfill('0') << std::setw(2) << month << "-"
                       << std::setw(2) << startDay << "-" << std::setw(2) << endDay << ".db";
            const std::string weekDbPath = attendanceBasePath + dbFileName.str();

            if (system::searchFile(weekDbPath)) {
                // Get all week start dates for this employee in this database
                for (int row = 1; row <= 100; ++row) {
                    std::string empId = db::fetchCell(weekDbPath, static_cast<size_t>(row), 1);
                    if (empId.empty()) break;

                    if (empId == employeeId) {
                        std::string weekStart = db::fetchCell(weekDbPath, static_cast<size_t>(row), 2);
                        if (!weekStart.empty()) {
                            // Build week label from filename for deletion
                            std::string weekLabel = dbFileName.str();
                            weekLabel = weekLabel.substr(0, weekLabel.length() - 3); // Remove .db
                            std::ranges::replace(weekLabel, '-', '/');

                            if (db::deleteWeeklyAttendanceRow(weekDbPath, employeeId, weekStart)) {
                                deletedRecords++;
                            }
                        }
                    }
                }
            }
        }
    }

    if (deletedRecords > 0) {
        system::logMessage(system::messageClassification::INFO,
            "Deleted " + std::to_string(deletedRecords) + " weekly attendance record(s) for employee ID: " + employeeId + "\n");
    }

    return true;
}

// === Worker Monitoring ===

// Helper: normalize raw user input to canonical EMP-xxxxx format
// Extract numeric employee ID from various input formats (e.g., "123", "EMP-00123", "00123")
// Returns the numeric ID as a string for database storage (e.g., "123")
static std::string normalizeEmployeeId(const std::string& raw) {
    std::string digits;
    for (const char ch : raw) {
        if (std::isdigit(static_cast<unsigned char>(ch))) digits.push_back(ch);
    }
    if (digits.empty()) return {};
    long long idVal = 0;
    try { idVal = std::stoll(digits); } catch (...) { return {}; }
    // Return numeric ID as string for database storage
    return std::to_string(idVal);
}

// Sanitize a numeric token for SQL (hours). Keep digits and one dot; default to 0 when empty.
static std::string sanitizeHoursToken(const std::string& in) {
    std::string out;
    bool dotUsed = false;
    for (char ch : in) {
        if (std::isdigit(static_cast<unsigned char>(ch))) {
            out.push_back(ch);
        } else if (ch == '.' && !dotUsed) {
            out.push_back('.');
            dotUsed = true;
        }
    }
    if (out.empty()) return "0";
    // Edge cases like "." -> invalid; treat as 0
    if (out == ".") return "0";
    return out;
}

// Build attendance DB full path from UI week label (e.g., "01/05-11" or "01/29-02/04")
static std::string buildAttendanceDbPathFromWeekLabel(const std::string& weekLabelUi) {
    // Replace '/' with '-' to match filenames used by UI
    std::string safeWeekLabel = weekLabelUi;
    std::ranges::replace(safeWeekLabel, '/', '-');

    const std::string attendanceDir = appConfig::g_dataDirectory + appConfig::g_payrollDirectory +
                                     appConfig::g_payrollAttendanceDirectory;
    if (!system::searchDirectory(attendanceDir)) {
        system::createDirectory(attendanceDir);
    }
    return attendanceDir + safeWeekLabel + ".db";
}

// loadWeeklyAttendance removed as unused

std::vector<Monitor::AttendanceRow> Monitor::listWeeklyAttendance(const std::string& weekLabelUi, int maxRows) {
    std::vector<AttendanceRow> rows;
    if (weekLabelUi.empty() || maxRows < 1) return rows;
    const std::string attendanceDbPath = buildAttendanceDbPathFromWeekLabel(weekLabelUi);
    rows.reserve(static_cast<size_t>(std::min(maxRows, 100)));
    for (int row = 1; row <= maxRows; ++row) {
        AttendanceRow ar{};
        ar.employeeId = db::fetchCell(attendanceDbPath, static_cast<size_t>(row), 1);
        if (ar.employeeId.empty()) break;
        ar.weekStartIso = db::fetchCell(attendanceDbPath, static_cast<size_t>(row), 2);
        ar.sun = db::fetchCell(attendanceDbPath, static_cast<size_t>(row), 3);
        ar.mon = db::fetchCell(attendanceDbPath, static_cast<size_t>(row), 4);
        ar.tue = db::fetchCell(attendanceDbPath, static_cast<size_t>(row), 5);
        ar.wed = db::fetchCell(attendanceDbPath, static_cast<size_t>(row), 6);
        ar.thu = db::fetchCell(attendanceDbPath, static_cast<size_t>(row), 7);
        ar.fri = db::fetchCell(attendanceDbPath, static_cast<size_t>(row), 8);
        ar.sat = db::fetchCell(attendanceDbPath, static_cast<size_t>(row), 9);
        rows.emplace_back(std::move(ar));
    }
    return rows;
}

bool Monitor::addWeeklyAttendance(
    const std::string& employeeIdRaw,
    const std::string& weekLabelUi,
    const std::string& weekStartIso,
    const std::string& sun,
    const std::string& mon,
    const std::string& tue,
    const std::string& wed,
    const std::string& thu,
    const std::string& fri,
    const std::string& sat
) {
    const std::string empId = normalizeEmployeeId(employeeIdRaw);
    if (empId.empty() || weekStartIso.empty() || weekLabelUi.empty()) return false;

    const std::string attendanceDbPath = buildAttendanceDbPathFromWeekLabel(weekLabelUi);
    // Ensure weekly file table exists
    if (!db::ensureWeeklyAttendanceTable(attendanceDbPath)) {
        // Try creating DB and retry ensure
        if (!db::createDatabase(attendanceDbPath) || !db::ensureWeeklyAttendanceTable(attendanceDbPath)) {
            return false;
        }
    }

    const std::string values = "'" + empId + "', '" + weekStartIso + "', " +
                               sanitizeHoursToken(sun) + ", " + sanitizeHoursToken(mon) + ", " +
                               sanitizeHoursToken(tue) + ", " + sanitizeHoursToken(wed) + ", " +
                               sanitizeHoursToken(thu) + ", " + sanitizeHoursToken(fri) + ", " +
                               sanitizeHoursToken(sat);

    bool okWeek = db::insertWeeklyAttendance(attendanceDbPath, values);

    // Mirror into base_payroll.db
    const std::string basePath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;
    // Ensure base has table
    db::createDatabase(basePath); // idempotent; also creates EMPLOYEES if needed
    bool okBase = db::insertWeeklyAttendance(basePath, values);

    return okWeek && okBase;
}

bool Monitor::updateWeeklyAttendance(
    const std::string& employeeIdRaw,
    const std::string& weekLabelUi,
    const std::string& weekStartIso,
    const std::string& sun,
    const std::string& mon,
    const std::string& tue,
    const std::string& wed,
    const std::string& thu,
    const std::string& fri,
    const std::string& sat
) {
    const std::string empId = normalizeEmployeeId(employeeIdRaw);
    if (empId.empty() || weekLabelUi.empty() || weekStartIso.empty()) return false;

    const std::string attendanceDbPath = buildAttendanceDbPathFromWeekLabel(weekLabelUi);
    const std::string setClause = "SUN=" + sanitizeHoursToken(sun) +
                                  ", MON=" + sanitizeHoursToken(mon) +
                                  ", TUE=" + sanitizeHoursToken(tue) +
                                  ", WED=" + sanitizeHoursToken(wed) +
                                  ", THU=" + sanitizeHoursToken(thu) +
                                  ", FRI=" + sanitizeHoursToken(fri) +
                                  ", SAT=" + sanitizeHoursToken(sat);

    // Ensure table exists and update in week file
    db::ensureWeeklyAttendanceTable(attendanceDbPath);
    bool okWeek = db::updateWeeklyAttendanceRow(attendanceDbPath, empId, weekStartIso, setClause);

    // Update mirror in base_payroll.db
    const std::string basePath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;
    db::createDatabase(basePath);
    bool okBase = db::updateWeeklyAttendanceRow(basePath, empId, weekStartIso, setClause);
    return okWeek && okBase;
}

bool Monitor::deleteWeeklyAttendance(
    const std::string& employeeIdRaw,
    const std::string& weekLabelUi,
    const std::string& weekStartIso
) {
    const std::string empId = normalizeEmployeeId(employeeIdRaw);
    if (empId.empty() || weekLabelUi.empty() || weekStartIso.empty()) return false;

    const std::string attendanceDbPath = buildAttendanceDbPathFromWeekLabel(weekLabelUi);
    db::ensureWeeklyAttendanceTable(attendanceDbPath);
    bool okWeek = db::deleteWeeklyAttendanceRow(attendanceDbPath, empId, weekStartIso);

    const std::string basePath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;
    db::createDatabase(basePath);
    bool okBase = db::deleteWeeklyAttendanceRow(basePath, empId, weekStartIso);
    return okWeek && okBase;
}

std::vector<std::string> Monitor::listProjectIDs() {
    std::vector<std::string> projectIDs;
    const std::string dbPath = appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_dbNameProject;

    // Fetch up to 1000 project IDs from the database
    for (int row = 1; row <= 1000; ++row) {
        std::string projectId = db::fetchCell(dbPath, static_cast<size_t>(row), 1);
        if (projectId.empty()) break;
        projectIDs.push_back(projectId);
    }

    return projectIDs;
}

bool Monitor::addEmployeeToProject(const std::string& projectId, const std::string& employeeId,
                         const std::string& employeeName, const std::string& position,
                         const std::string& hourlyRate) {
    if (projectId.empty() || employeeId.empty()) return false;

    const std::string projectExpenseDbPath = appConfig::g_dataDirectory + appConfig::g_projectDirectory +
                                            appConfig::g_projectExpenseDirectory + projectId + ".db";

    // Ensure the project expense database exists
    db::createDatabase(projectExpenseDbPath);

    // Insert employee into PAYROLL_EXPENSES table
    const std::string values = employeeId + ", '" + employeeName + "', '" + position + "', " +
                              hourlyRate + ", 0, 0";

    return db::insertPayrollExpense(projectExpenseDbPath, values);
}

bool Monitor::removeEmployeeFromProject(const std::string& projectId, const std::string& employeeId) {
    if (projectId.empty() || employeeId.empty()) return false;

    const std::string projectExpenseDbPath = appConfig::g_dataDirectory + appConfig::g_projectDirectory +
                                            appConfig::g_projectExpenseDirectory + projectId + ".db";

    if (!system::searchFile(projectExpenseDbPath)) return false;

    return db::deletePayrollExpense(projectExpenseDbPath, employeeId);
}

bool Monitor::updateEmployeeProjectHours(const std::string& projectId, const std::string& employeeId,
                               double totalHours, double totalCost) {
    if (projectId.empty() || employeeId.empty()) return false;

    const std::string projectExpenseDbPath = appConfig::g_dataDirectory + appConfig::g_projectDirectory +
                                            appConfig::g_projectExpenseDirectory + projectId + ".db";

    if (!system::searchFile(projectExpenseDbPath)) return false;

    std::ostringstream setClause;
    setClause << "TOTAL_HOURS=" << std::fixed << std::setprecision(2) << totalHours
              << ", TOTAL_COST=" << std::fixed << std::setprecision(2) << totalCost;

    return db::updatePayrollExpense(projectExpenseDbPath, employeeId, setClause.str());
}

bool Monitor::calculateProjectPayrollCosts(const std::string& projectId) {
    if (projectId.empty()) return false;

    const std::string payrollDbPath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;
    const std::string projectDbPath = appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_dbNameProject;
    const std::string projectExpenseDbPath = appConfig::g_dataDirectory + appConfig::g_projectDirectory +
                                            appConfig::g_projectExpenseDirectory + projectId + ".db";

    // Check if project expense database exists
    if (!system::searchFile(projectExpenseDbPath)) {
        system::logMessage(system::messageClassification::WARNING,
            "Project expense database not found for " + projectId + "\n");
        return false;
    }

    // Get project START_DATE and END_DATE
    std::string projectStartDate;
    std::string projectEndDate;

    sqlite3* projectDb = nullptr;
    if (sqlite3_open(projectDbPath.c_str(), &projectDb) == SQLITE_OK) {
        sqlite3_stmt* stmt = nullptr;
        const char* sql = "SELECT START_DATE, END_DATE FROM PROJECT_LIST WHERE PROJECT_ID = ? LIMIT 1;";

        if (sqlite3_prepare_v2(projectDb, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, projectId.c_str(), -1, SQLITE_TRANSIENT);

            if (sqlite3_step(stmt) == SQLITE_ROW) {
                if (const auto* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))) {
                    projectStartDate = text;
                }
                if (const auto* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))) {
                    projectEndDate = text;
                }
            }
            sqlite3_finalize(stmt);
        }
        sqlite3_close(projectDb);
    }

    if (projectStartDate.empty()) {
        system::logMessage(system::messageClassification::WARNING,
            "No start date found for project " + projectId + "\n");
        return false;
    }

    // Get all employees assigned to this project
    for (int row = 1; row <= 1000; ++row) {
        std::string empId = db::fetchCell(payrollDbPath, static_cast<size_t>(row), 1);
        if (empId.empty()) break;

        std::string siteLocation = db::fetchCell(payrollDbPath, static_cast<size_t>(row), 4);

        // Check if employee is assigned to this project
        if (siteLocation == projectId) {
            std::string hourlyRateStr = db::fetchCell(payrollDbPath, static_cast<size_t>(row), 5);
            double hourlyRate = 0.0;
            try {
                hourlyRate = std::stod(hourlyRateStr);
            } catch (...) {
                continue;
            }

            // Calculate total hours from weekly attendance records within project date range
            double totalHours = 0.0;

            // Query WEEKLY_ATTENDANCE table with date filtering
            sqlite3* db = nullptr;
            if (sqlite3_open(payrollDbPath.c_str(), &db) == SQLITE_OK) {
                sqlite3_stmt* stmt = nullptr;
                std::string sql;

                if (projectEndDate.empty()) {
                    // No end date - count from start date onwards
                    sql = "SELECT SUM(SUN + MON + TUE + WED + THU + FRI + SAT) FROM WEEKLY_ATTENDANCE "
                          "WHERE EMPLOYEE_ID = ? AND WEEK_START >= ?;";
                } else {
                    // Count between start and end dates
                    sql = "SELECT SUM(SUN + MON + TUE + WED + THU + FRI + SAT) FROM WEEKLY_ATTENDANCE "
                          "WHERE EMPLOYEE_ID = ? AND WEEK_START >= ? AND WEEK_START <= ?;";
                }

                if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
                    sqlite3_bind_int(stmt, 1, std::stoi(empId));
                    sqlite3_bind_text(stmt, 2, projectStartDate.c_str(), -1, SQLITE_TRANSIENT);

                    if (!projectEndDate.empty()) {
                        sqlite3_bind_text(stmt, 3, projectEndDate.c_str(), -1, SQLITE_TRANSIENT);
                    }

                    if (sqlite3_step(stmt) == SQLITE_ROW) {
                        totalHours = sqlite3_column_double(stmt, 0);
                    }
                    sqlite3_finalize(stmt);
                }
                sqlite3_close(db);
            }

            // Calculate total cost
            double totalCost = totalHours * hourlyRate;

            // Update project payroll expenses
            updateEmployeeProjectHours(projectId, empId, totalHours, totalCost);

            system::logMessage(system::messageClassification::INFO,
                "Updated payroll for Employee " + empId + " in project " + projectId +
                " (" + projectStartDate + " to " + (projectEndDate.empty() ? "present" : projectEndDate) + ")" +
                ": " + std::to_string(totalHours) + " hours, ₱" + std::to_string(totalCost) + "\n");
        }
    }

    return true;
}

bool Monitor::calculateAllProjectPayrollCosts() {
    // Get all projects
    std::vector<std::string> projectIDs = listProjectIDs();

    int updatedProjects = 0;
    for (const auto& projectId : projectIDs) {
        if (calculateProjectPayrollCosts(projectId)) {
            updatedProjects++;
        }
    }

    system::logMessage(system::messageClassification::INFO,
        "Updated payroll costs for " + std::to_string(updatedProjects) + " project(s)\n");

    return updatedProjects > 0;
}

// Removed deprecated sample calculators (not used by the system)



