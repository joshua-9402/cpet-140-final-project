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
#include "monitor.h"
#include "../handler/db.h"
#include "../config/config.h"


bool addProject(const std::string& p_projectName, const std::string& p_status, const std::string& p_startDate, const std::string& p_note = "") {
    if (db::appendDatabase(
                appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_dbNameProject,
         "\"'" + p_projectName + "', '" + p_status + "', '" + p_startDate + "', '" + p_note + "'\"")
        &&
        db::createDatabase(appConfig::g_dataDirectory + appConfig::g_projectDirectory + appConfig::g_projectExpenseDirectory + p_projectName + ".db")
        ) {return true;}

    return false;
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

