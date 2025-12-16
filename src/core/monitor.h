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

    // Worker monitoring helpers (dummy/sample data providers)
    void loadWorkerAttendance(
        std::string days[7],
        std::string timeIn[7],
        std::string timeOut[7]
    );

    // Load weekly attendance hours for a specific employee and week label (e.g., "01/05-11").
    // Returns true if found and fills outHours in order: SUN, MON, TUE, WED, THU, FRI, SAT.
    bool loadWeeklyAttendance(
        const std::string& employeeIdRaw,
        const std::string& weekLabelUi,
        std::string outHours[7]
    );

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

    // Equipment monitoring calculator (sample)
    double computeEquipmentCost(
        std::string eqNames[3],
        double hours[3],
        double rate[3],
        double totalOut[3],
        std::string eqIDs[3]
    );

    // Project monitoring calculator (sample)
    double computeProjectExpenses(
        std::string projIDs[3],
        std::string projNames[3],
        std::string status[3],
        std::string startDate[3],
        double labor[3],
        double equip[3],
        double materials[3],
        double totalOut[3]
    );
}
#endif
