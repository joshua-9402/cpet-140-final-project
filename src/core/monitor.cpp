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

#include <iostream>
#include <vector>
#include <string>
#include <numeric>
#include <iomanip>

using namespace std;

// -----------------------------
// Data Structures
// -----------------------------
struct Expense {
    string category;
    double amount;
};

struct PayrollEntry {
    string employeeName;
    double salary;
    double deductions;
};

// -----------------------------
// Monitoring System Class
// -----------------------------
class MonitoringSystem {
private:
    vector<Expense> expenses;
    vector<PayrollEntry> payroll;

public:
    // Add domain-specific expense
    void addExpense(const string& category, double amount) {
        expenses.push_back({category, amount});
    }

    // Add payroll entry
    void addPayroll(const string& name, double salary, double deductions) {
        payroll.push_back({name, salary, deductions});
    }

    // Expose domain-specific metrics
    void showExpenseMetrics() {
        cout << "\n=== Expense Metrics ===\n";
        for (const auto& e : expenses) {
            cout << "Category: " << e.category
                 << " | Amount: ₱" << fixed << setprecision(2) << e.amount << "\n";
        }
    }

    void showPayrollMetrics() {
        cout << "\n=== Payroll Metrics ===\n";
        for (const auto& p : payroll) {
            cout << "Employee: " << p.employeeName
                 << " | Salary: ₱" << fixed << setprecision(2) << p.salary
                 << " | Deductions: ₱" << p.deductions << "\n";
        }
    }

    // Summaries
    double getTotalExpenses() {
        return accumulate(expenses.begin(), expenses.end(), 0.0,
            [](double sum, const Expense& e) { return sum + e.amount; });
    }

    double getTotalPayrollOutflow() {
        return accumulate(payroll.begin(), payroll.end(), 0.0,
            [](double sum, const PayrollEntry& p) { return sum + (p.salary - p.deductions); });
    }

    void showSummary() {
        cout << "\n=== Summary Report ===\n";
        cout << "Total Expenses: ₱" << fixed << setprecision(2) << getTotalExpenses() << "\n";
        cout << "Total Payroll Outflow: ₱" << getTotalPayrollOutflow() << "\n";
        cout << "Overall Outflow: ₱"
             << getTotalExpenses() + getTotalPayrollOutflow() << "\n";
    }
};

// -----------------------------
// Main Program
// -----------------------------
// int main() {
//     MonitoringSystem system;
//
//     // Sample data (you can replace with real inputs)
//     system.addExpense("Office Supplies", 4500.75);
//     system.addExpense("Utilities", 12000.00);
//     system.addExpense("Maintenance", 3500.25);
//
//     system.addPayroll("Alice Santos", 30000, 2500);
//     system.addPayroll("Mark Dela Cruz", 28000, 2000);
//     system.addPayroll("Jessa Ramirez", 32000, 3000);
//
//     // Display metrics
//     system.showExpenseMetrics();
//     system.showPayrollMetrics();
//
//     // Display summaries
//     system.showSummary();
//
//     return 0;
// }
