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
#include <map>

namespace monitor {

    // Data structures for monitoring
    struct ExpenseRecord {
        std::string projectName;
        std::string category;
        double amount;
        std::string date;
        std::string description;
    };

    struct PayrollSummary {
        std::string period;
        double totalGrossPay;
        double totalDeductions;
        double totalNetPay;
        int employeeCount;
    };

    struct ProjectMetrics {
        std::string projectName;
        double totalExpenses;
        double budgetAllocated;
        double budgetRemaining;
        int transactionCount;
    };

    struct AlertThreshold {
        std::string metric;
        double threshold;
        bool isPercentage;
    };

    // Expense tracking functions
    bool addExpense(const ExpenseRecord& expense);
    std::vector<ExpenseRecord> getExpensesByProject(const std::string& projectName);
    std::vector<ExpenseRecord> getExpensesByDateRange(const std::string& startDate, const std::string& endDate);
    double getTotalExpensesByProject(const std::string& projectName);
    double getTotalExpensesByCategory(const std::string& category);

    // Payroll summary functions
    PayrollSummary getPayrollSummaryForPeriod(const std::string& period);
    std::vector<PayrollSummary> getPayrollHistory(int months);
    double getTotalPayrollCost(const std::string& startDate, const std::string& endDate);

    // Project metrics functions
    ProjectMetrics getProjectMetrics(const std::string& projectName);
    std::vector<ProjectMetrics> getAllProjectMetrics();
    bool updateProjectBudget(const std::string& projectName, double newBudget);
    double calculateBudgetUtilization(const std::string& projectName);

    // Alert and threshold functions
    bool setAlertThreshold(const AlertThreshold& threshold);
    std::vector<std::string> checkThresholds();
    bool isOverBudget(const std::string& projectName);

    // Reporting functions
    std::string generateExpenseReport(const std::string& startDate, const std::string& endDate);
    std::string generatePayrollReport(const std::string& period);
    std::string generateProjectReport(const std::string& projectName);

    // Data export functions
    bool exportExpensesToCSV(const std::string& filepath, const std::string& startDate, const std::string& endDate);
    bool exportPayrollToCSV(const std::string& filepath, const std::string& period);

    // Database integration functions (using db.h)
    bool initializeMonitoringDatabase(const std::string& dbName);
    bool saveExpenseToDatabase(const std::string& dbName, const ExpenseRecord& expense);
    bool savePayrollToDatabase(const std::string& dbName, const PayrollSummary& summary);
    bool saveProjectBudgetToFile(const std::string& filename, const std::string& projectName, double budget);
    double loadProjectBudgetFromFile(const std::string& filename, const std::string& projectName, int lineNumber);
    bool saveAllExpensesToDatabase(const std::string& dbName);

} // namespace monitor

#endif // MONITOR_H

