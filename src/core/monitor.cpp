/*
 * CpET 140 Final Project — Monitoring / Reporting module
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
 * - No UI rendering and n DB, or I/O operations.
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
#include <cctype>
#include "monitor.h"
#include "../handler/db.h"
#include "../config/config.h"
#include "../handler/system.h"


namespace monitor {

bool addProject(const std::string& projectId,
                const std::string& projectName,
                const std::string& status,
                const std::string& startDate,
                const std::string& note) {
    const std::string dbPath = appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_dbNameProject;
    const std::string values = "'" + projectId + "', '" + projectName + "', '" + status + "', '" + startDate + "', '" + note + "'";

    if (db::appendDatabase(dbPath, values)
        && db::createDatabase(appConfig::g_dataDirectory + appConfig::g_projectDirectory +
                              appConfig::g_projectExpenseDirectory + projectId + ".db")) {
        return true;
    }
    return false;
}


PayrollSummary computePayrollSummary() {
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

ProjectSummary computeProjectSummary() {
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

bool addEmployee(const std::string& name,
                 const std::string& position,
                 const std::string& siteLocation,
                 const std::string& hourlyRate,
                 const std::string& hoursWorked,
                 const std::string& advance) {
    const std::string dbPath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;
    const std::string values = "'" + name + "', '" + position + "', '" + siteLocation + "', " + hourlyRate + ", " + hoursWorked + ", " + advance;
    return db::appendDatabase(dbPath, values);
}

bool updateEmployee(const std::string& newEmployeeId,
                    const std::string& name,
                    const std::string& position,
                    const std::string& siteLocation,
                    const std::string& hourlyRate,
                    const std::string& hoursWorked,
                    const std::string& advance) {
    if (newEmployeeId.empty()) return false;

    const std::string payrollDbPath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;

    std::string oldEmployeeID;
    bool foundEmployee = false;

    // Try to find an exact match by ID first
    for (int row = 1; row <= 1000; ++row) {
        std::string currentID = db::fetchCell(payrollDbPath, static_cast<size_t>(row), 1);
        if (currentID.empty()) break;
        if (currentID == newEmployeeId) {
            oldEmployeeID = currentID;
            foundEmployee = true;
            break;
        }
    }

    // Otherwise try to find by name (best-effort)
    if (!foundEmployee && !name.empty()) {
        for (int row = 1; row <= 1000; ++row) {
            std::string currentID = db::fetchCell(payrollDbPath, static_cast<size_t>(row), 1);
            if (currentID.empty()) break;
            std::string currentName = db::fetchCell(payrollDbPath, static_cast<size_t>(row), 2);
            if (currentName == name) {
                oldEmployeeID = currentID;
                foundEmployee = true;
                break;
            }
        }
    }

    // If ID changed, propagate to attendance databases of current year
    if (foundEmployee && oldEmployeeID != newEmployeeId) {
        const int currentYear = system::fetchTime(system::PartDateTime::YEAR);
        const std::string attendanceBasePath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + std::to_string(currentYear) + "/";

        for (int month = 1; month <= 12; month++) {
            for (int day = 1; day <= 31; day += 7) {
                int endDay = std::min(day + 6, 31);
                std::ostringstream dbFileName;
                dbFileName << std::setfill('0') << std::setw(2) << month << "-"
                           << std::setw(2) << day << "-" << std::setw(2) << endDay << ".db";
                const std::string attendanceDbPath = attendanceBasePath + dbFileName.str();
                if (system::searchFile(attendanceDbPath)) {
                    const std::string setClause = "EMPLOYEE_ID='" + newEmployeeId + "'";
                    db::updateDatabase(attendanceDbPath, oldEmployeeID, setClause);
                }
            }
        }
    }

    const std::string updateKey = foundEmployee ? oldEmployeeID : newEmployeeId;
    const std::string setClause = "EMPLOYEE_ID='" + newEmployeeId + "', NAME='" + name + "', POSITION='" + position +
                                  "', SITE_LOCATION='" + siteLocation + "', SALARY=" + hourlyRate + ", HOURS_WORK=" + hoursWorked + ", ADVANCE=" + advance;
    return db::updateDatabase(payrollDbPath, updateKey, setClause);
}

bool deleteEmployee(const std::string& employeeId) {
    const std::string dbPath = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + appConfig::g_dbNamePayroll;
    return db::deleteRow(dbPath, employeeId);
}

// === Worker Monitoring ===
void loadWorkerAttendance(
    std::string days[7],
    std::string timeIn[7],
    std::string timeOut[7]
) {
    days[0] = "Monday";    timeIn[0] = "0800";  timeOut[0] = "1600";
    days[1] = "Tuesday";   timeIn[1] = "0700";  timeOut[1] = "1100";
    days[2] = "Wednesday"; timeIn[2] = "0800";  timeOut[2] = "1500";
    days[3] = "Thursday";  timeIn[3] = "0700";  timeOut[3] = "1800";
    days[4] = "Friday";    timeIn[4] = "1000";  timeOut[4] = "1800";
    days[5] = "Saturday";  timeIn[5] = "0630";  timeOut[5] = "1130";
    days[6] = "Sunday";    timeIn[6] = "0100";  timeOut[6] = "0300";
}

// Helper: normalize raw user input to canonical EMP-xxxxx format
static std::string normalizeEmployeeId(const std::string& raw) {
    std::string digits;
    for (char ch : raw) {
        if (std::isdigit(static_cast<unsigned char>(ch))) digits.push_back(ch);
    }
    if (digits.empty()) return {};
    long long idVal = 0;
    try { idVal = std::stoll(digits); } catch (...) { return {}; }
    std::ostringstream oss;
    oss << "EMP-" << std::setw(5) << std::setfill('0') << idVal;
    return oss.str();
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
    std::replace(safeWeekLabel.begin(), safeWeekLabel.end(), '/', '-');

    const int currentYear = system::fetchTime(system::PartDateTime::YEAR);
    const std::string yearDir = appConfig::g_dataDirectory + appConfig::g_payrollDirectory + std::to_string(currentYear);
    if (!system::searchDirectory(yearDir)) {
        system::createDirectory(yearDir);
    }
    return yearDir + "/" + safeWeekLabel + ".db";
}

bool loadWeeklyAttendance(
    const std::string& employeeIdRaw,
    const std::string& weekLabelUi,
    std::string outHours[7]
) {
    const std::string empId = normalizeEmployeeId(employeeIdRaw);
    if (empId.empty() || weekLabelUi.empty()) return false;

    const std::string attendanceDbPath = buildAttendanceDbPathFromWeekLabel(weekLabelUi);
    for (int row = 1; row <= 1000; ++row) {
        const std::string idCell = db::fetchCell(attendanceDbPath, static_cast<size_t>(row), 1);
        if (idCell.empty()) break;
        if (idCell == empId) {
            // Columns: 1=EMPLOYEE_ID, 2=WEEK_START, 3..9=SUN..SAT
            for (int i = 0; i < 7; ++i) {
                outHours[i] = db::fetchCell(attendanceDbPath, static_cast<size_t>(row), static_cast<size_t>(3 + i));
            }
            return true;
        }
    }
    return false;
}

bool addWeeklyAttendance(
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

bool updateWeeklyAttendance(
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

bool deleteWeeklyAttendance(
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

// === Equipment Monitoring ===
double computeEquipmentCost(
    std::string eqNames[3],
    double hours[3],
    double rate[3],
    double totalOut[3],
    std::string eqIDs[3]
) {
    eqIDs[0] = "EQP-001"; eqNames[0] = "EQP-001"; hours[0] = 10; rate[0] = 500;
    eqIDs[1] = "EQP-002"; eqNames[1] = "EQP-002";     hours[1] = 8;  rate[1] = 700;
    eqIDs[2] = "EQP-003"; eqNames[2] = "EQP-003";     hours[2] = 12; rate[2] = 300;

    double totalCost = 0;
    for (int i = 0; i < 3; i++) {
        totalOut[i] = hours[i] * rate[i];
        totalCost += totalOut[i];
    }
    return totalCost;
}

// === Project Monitoring ===
double computeProjectExpenses(
    std::string projIDs[3],
    std::string projNames[3],
    std::string status[3],
    std::string startDate[3],
    double labor[3],
    double equip[3],
    double materials[3],
    double totalOut[3]
) {
    projIDs[0] = "PRJ-00001"; projNames[0] = "MinSU Dormitory";
    status[0] = "In Progress"; startDate[0] = "2025-01-15";
    labor[0] = 20000; equip[0] = 15000; materials[0] = 10000;

    projIDs[1] = "PRJ-00002"; projNames[1] = "MinSU Dormitory";
    status[1] = "Completed"; startDate[1] = "2024-11-01";
    labor[1] = 30000; equip[1] = 20000; materials[1] = 15000;

    projIDs[2] = "PRJ-00003"; projNames[2] = "MinSU Dormitory";
    status[2] = "Active"; startDate[2] = "2023-10-25";
    labor[2] = 25000; equip[2] = 18000; materials[2] = 12000;

    double grand = 0;
    for (int i = 0; i < 3; i++) {
        totalOut[i] = labor[i] + equip[i] + materials[i];
        grand += totalOut[i];
    }
    return grand;
}

} // namespace monitor

