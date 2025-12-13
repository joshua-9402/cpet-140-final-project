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
using namespace std;

// === Worker Monitoring (NO PRINTING) ===
void loadWorkers(
    string days[7],
    string timeIn[7],
    string timeOut[7]
) {
    days[0]="Monday";    timeIn[0]="800";  timeOut[0]="400";
    days[1]="Tuesday";   timeIn[1]="700";  timeOut[1]="1100";
    days[2]="Wednesday"; timeIn[2]="800";  timeOut[2]="300";
    days[3]="Thursday";  timeIn[3]="700";  timeOut[3]="600";
    days[4]="Friday";    timeIn[4]="1000"; timeOut[4]="600";
    days[5]="Saturday";  timeIn[5]="630";  timeOut[5]="1130";
    days[6]="Sunday";    timeIn[6]="100";  timeOut[6]="300";
}

// === Equipment Monitoring (NO PRINTING) ===
double computeEquipmentCost(
    string eqNames[3],
    double hours[3],
    double rate[3],
    double totalOut[3]
) {
    eqNames[0]="Equipment 1"; hours[0]=10; rate[0]=500;
    eqNames[1]="Equipment 2"; hours[1]=8;  rate[1]=700;
    eqNames[2]="Equipment 3"; hours[2]=12; rate[2]=300;

    double totalCost = 0;

    for (int i = 0; i < 3; i++) {
        totalOut[i] = hours[i] * rate[i];
        totalCost += totalOut[i];
    }

    return totalCost;
}

// === Project Monitoring (NO PRINTING) ===
double computeProjectExpenses(
    string projNames[3],
    double labor[3],
    double equip[3],
    double materials[3],
    double totalOut[3]
) {
    projNames[0]="Project 1"; labor[0]=20000; equip[0]=15000; materials[0]=10000;
    projNames[1]="Project 2"; labor[1]=30000; equip[1]=20000; materials[1]=15000;
    projNames[2]="Project 3"; labor[2]=25000; equip[2]=18000; materials[2]=12000;

    double grand = 0;

    for (int i = 0; i < 3; i++) {
        totalOut[i] = labor[i] + equip[i] + materials[i];
        grand += totalOut[i];
    }

    return grand;
}
