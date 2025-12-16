/*
 * CpET 140 Final Project — Monitoring / Reporting module header
 *
 * Contributors:
 *  Hershey Mae Tenorio
 *  Mary Grace Jusay
 *  Matthew Anton Guina
 *
 * Purpose
 * - Define monitoring data structures and function prototypes
 */

#ifndef MONITOR_H
#define MONITOR_H

#include <string>
#include <vector>
struct PayrollSummary {
    int totalEmployees;
    double totalSalaryExpense;
    double totalAdvances;
};

struct ProjectSummary {
    int totalProjects;
    int activeProjects;
};

namespace monitor {
    // Lightweight row for employee listings
    struct EmployeeRow {
        std::string id;
        std::string name;
        std::string position;
        std::string siteLocation;
        std::string salary;
        std::string hoursWork;
        std::string advance;
    };
    // Project management
    bool addProject(
        const std::string& projectId,
        const std::string& projectName,
        const std::string& status,
        const std::string& startDate,
        const std::string& note = ""
    );

    // Payroll/Employee summaries (DB-backed aggregations)
    PayrollSummary computePayrollSummary();
    ProjectSummary computeProjectSummary();

    // Employee management (DB-backed)
    bool addEmployee(
        const std::string& name,
        const std::string& position,
        const std::string& siteLocation,
        const std::string& hourlyRate,
        const std::string& hoursWorked,
        const std::string& advance
    );

    // Detects ID change and propagates to attendance DBs automatically
    bool updateEmployee(
        const std::string& newEmployeeId,
        const std::string& name,
        const std::string& position,
        const std::string& siteLocation,
        const std::string& hourlyRate,
        const std::string& hoursWorked,
        const std::string& advance
    );

    bool deleteEmployee(const std::string& employeeId);

    // Employee listings for UI (DB-backed)
    std::vector<EmployeeRow> listEmployees(int maxRows = 1000);

    struct AttendanceRow {
        std::string employeeId;
        std::string weekStartIso;
        std::string sun;
        std::string mon;
        std::string tue;
        std::string wed;
        std::string thu;
        std::string fri;
        std::string sat;
    };

    // Weekly attendance listing for viewer
    std::vector<AttendanceRow> listWeeklyAttendance(const std::string& weekLabelUi, int maxRows = 100);

    // Weekly attendance operations (single-call APIs for UI buttons)
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
    );

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
    );

    bool deleteWeeklyAttendance(
        const std::string& employeeIdRaw,
        const std::string& weekLabelUi,
        const std::string& weekStartIso
    );

    // Deprecated sample calculators removed (not used by system)
}
#endif
