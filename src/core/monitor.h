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

namespace monitor {
    // Project management
    bool addProject(
        const std::string& projectId,
        const std::string& projectName,
        const std::string& status,
        const std::string& startDate,
        const std::string& note = ""
    );

    // Worker monitoring helpers (dummy/sample data providers)
    void loadWorkerAttendance(
        std::string days[7],
        std::string timeIn[7],
        std::string timeOut[7]
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
