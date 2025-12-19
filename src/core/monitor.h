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

class Monitor {
public:
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

    // Project management
    static bool addProject(
        const std::string& projectId,
        const std::string& projectName,
        const std::string& status,
        const std::string& startDate,
        const std::string& endDate = "",
        const std::string& note = ""
    );

    // Payroll/Employee summaries (DB-backed aggregations)
    static PayrollSummary computePayrollSummary();
    static ProjectSummary computeProjectSummary();

    // Employee management (DB-backed)
    static bool addEmployee(
        const std::string& name,
        const std::string& position,
        const std::string& siteLocation,
        const std::string& hourlyRate,
        const std::string& hoursWorked,
        const std::string& advance
    );

    // Detects ID change and propagates to attendance DBs automatically
    static bool updateEmployee(
        const std::string& newEmployeeId,
        const std::string& name,
        const std::string& position,
        const std::string& siteLocation,
        const std::string& hourlyRate,
        const std::string& hoursWorked,
        const std::string& advance
    );

    static bool deleteEmployee(const std::string& employeeId);

    // Employee listings for UI (DB-backed)
    static std::vector<EmployeeRow> listEmployees(int maxRows = 1000);

    // Weekly attendance listing for viewer
    static std::vector<AttendanceRow> listWeeklyAttendance(const std::string& weekLabelUi, int maxRows = 100);

    // Weekly attendance operations (single-call APIs for UI buttons)
    static bool addWeeklyAttendance(
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

    static bool updateWeeklyAttendance(
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

    static bool deleteWeeklyAttendance(
        const std::string& employeeIdRaw,
        const std::string& weekLabelUi,
        const std::string& weekStartIso
    );

    // Get list of all project IDs for site location dropdown
    static std::vector<std::string> listProjectIDs();

    // Manage employee payroll expenses in projects
    static bool addEmployeeToProject(const std::string& projectId, const std::string& employeeId,
                             const std::string& employeeName, const std::string& position,
                             const std::string& hourlyRate);
    static bool removeEmployeeFromProject(const std::string& projectId, const std::string& employeeId);
    static bool updateEmployeeProjectHours(const std::string& projectId, const std::string& employeeId,
                                   double totalHours, double totalCost);

    // Calculate and update project payroll costs from weekly attendance
    static bool calculateProjectPayrollCosts(const std::string& projectId);
    static bool calculateAllProjectPayrollCosts(); // Update all projects
};

// Backward compatibility
namespace monitor {
    using EmployeeRow = Monitor::EmployeeRow;
    using AttendanceRow = Monitor::AttendanceRow;

    inline bool addProject(const std::string& projectId, const std::string& projectName,
                          const std::string& status, const std::string& startDate,
                          const std::string& endDate = "", const std::string& note = "") {
        return Monitor::addProject(projectId, projectName, status, startDate, endDate, note);
    }

    inline PayrollSummary computePayrollSummary() { return Monitor::computePayrollSummary(); }
    inline ProjectSummary computeProjectSummary() { return Monitor::computeProjectSummary(); }

    inline bool addEmployee(const std::string& name, const std::string& position,
                           const std::string& siteLocation, const std::string& hourlyRate,
                           const std::string& hoursWorked, const std::string& advance) {
        return Monitor::addEmployee(name, position, siteLocation, hourlyRate, hoursWorked, advance);
    }

    inline bool updateEmployee(const std::string& newEmployeeId, const std::string& name,
                              const std::string& position, const std::string& siteLocation,
                              const std::string& hourlyRate, const std::string& hoursWorked,
                              const std::string& advance) {
        return Monitor::updateEmployee(newEmployeeId, name, position, siteLocation, hourlyRate, hoursWorked, advance);
    }

    inline bool deleteEmployee(const std::string& employeeId) { return Monitor::deleteEmployee(employeeId); }
    inline std::vector<EmployeeRow> listEmployees(int maxRows = 1000) { return Monitor::listEmployees(maxRows); }
    inline std::vector<AttendanceRow> listWeeklyAttendance(const std::string& weekLabelUi, int maxRows = 100) {
        return Monitor::listWeeklyAttendance(weekLabelUi, maxRows);
    }

    inline bool addWeeklyAttendance(const std::string& employeeIdRaw, const std::string& weekLabelUi,
                                    const std::string& weekStartIso, const std::string& sun,
                                    const std::string& mon, const std::string& tue, const std::string& wed,
                                    const std::string& thu, const std::string& fri, const std::string& sat) {
        return Monitor::addWeeklyAttendance(employeeIdRaw, weekLabelUi, weekStartIso, sun, mon, tue, wed, thu, fri, sat);
    }

    inline bool updateWeeklyAttendance(const std::string& employeeIdRaw, const std::string& weekLabelUi,
                                       const std::string& weekStartIso, const std::string& sun,
                                       const std::string& mon, const std::string& tue, const std::string& wed,
                                       const std::string& thu, const std::string& fri, const std::string& sat) {
        return Monitor::updateWeeklyAttendance(employeeIdRaw, weekLabelUi, weekStartIso, sun, mon, tue, wed, thu, fri, sat);
    }

    inline bool deleteWeeklyAttendance(const std::string& employeeIdRaw, const std::string& weekLabelUi,
                                       const std::string& weekStartIso) {
        return Monitor::deleteWeeklyAttendance(employeeIdRaw, weekLabelUi, weekStartIso);
    }

    inline std::vector<std::string> listProjectIDs() { return Monitor::listProjectIDs(); }

    inline bool addEmployeeToProject(const std::string& projectId, const std::string& employeeId,
                                     const std::string& employeeName, const std::string& position,
                                     const std::string& hourlyRate) {
        return Monitor::addEmployeeToProject(projectId, employeeId, employeeName, position, hourlyRate);
    }

    inline bool removeEmployeeFromProject(const std::string& projectId, const std::string& employeeId) {
        return Monitor::removeEmployeeFromProject(projectId, employeeId);
    }

    inline bool updateEmployeeProjectHours(const std::string& projectId, const std::string& employeeId,
                                           double totalHours, double totalCost) {
        return Monitor::updateEmployeeProjectHours(projectId, employeeId, totalHours, totalCost);
    }

    inline bool calculateProjectPayrollCosts(const std::string& projectId) {
        return Monitor::calculateProjectPayrollCosts(projectId);
    }

    inline bool calculateAllProjectPayrollCosts() { return Monitor::calculateAllProjectPayrollCosts(); }
}

#endif
