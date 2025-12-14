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
namespace monitor {
    bool addProject(
        const std::string& p_projectName,
        const std::string& p_status,
        const std::string& p_startDate,
        const std::string& p_note = ""
    );
}
#endif
