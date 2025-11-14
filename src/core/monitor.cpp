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
 * - No UI rendering and no DB, or I/O operations.
 * - Persistence or queries must go through the db adapter (db.h) or other handler / service modules.
 * - No system-level monitoring (CPU/memory) here.
 *
 * Notes
 * - Single-threaded by default; callers must synchronize if used from workers.
 * - Validate inputs and avoid partial updates on failure. Emit alerts/logs when thresholds are breached.
 * - Keep everything simple and focused on core monitoring/reporting logic.
 *
 */

#include "monitor.h"
#include "../handler/db.h"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iomanip>

namespace monitor {

    // In-memory storage for monitoring data
    static std::vector<ExpenseRecord> g_expenses;
    static std::map<std::string, double> g_projectBudgets;
    static std::vector<AlertThreshold> g_alertThresholds;
    static std::vector<PayrollSummary> g_payrollHistory;

    // ============================================================================
    // EXPENSE TRACKING - Per-project expense totals
    // ============================================================================

    bool addExpense(const ExpenseRecord& expense) {
        // Validate input
        if (expense.projectName.empty() || expense.amount <= 0.0) {
            return false;
        }

        g_expenses.push_back(expense);

        // Check thresholds after adding expense
        checkThresholds();

        return true;
    }

    std::vector<ExpenseRecord> getExpensesByProject(const std::string& projectName) {
        std::vector<ExpenseRecord> result;

        for (const auto& expense : g_expenses) {
            if (expense.projectName == projectName) {
                result.push_back(expense);
            }
        }

        return result;
    }

    std::vector<ExpenseRecord> getExpensesByDateRange(const std::string& startDate, const std::string& endDate) {
        std::vector<ExpenseRecord> result;

        for (const auto& expense : g_expenses) {
            if (expense.date >= startDate && expense.date <= endDate) {
                result.push_back(expense);
            }
        }

        return result;
    }

    double getTotalExpensesByProject(const std::string& projectName) {
        double total = 0.0;

        for (const auto& expense : g_expenses) {
            if (expense.projectName == projectName) {
                total += expense.amount;
            }
        }

        return total;
    }

    double getTotalExpensesByCategory(const std::string& category) {
        double total = 0.0;

        for (const auto& expense : g_expenses) {
            if (expense.category == category) {
                total += expense.amount;
            }
        }

        return total;
    }

    // ============================================================================
    // PAYROLL MONITORING - Payroll/outflow summaries
    // ============================================================================

    PayrollSummary getPayrollSummaryForPeriod(const std::string& period) {
        // Search for existing summary
        for (const auto& summary : g_payrollHistory) {
            if (summary.period == period) {
                return summary;
            }
        }

        // Return empty summary if not found
        return PayrollSummary{"", 0.0, 0.0, 0.0, 0};
    }

    std::vector<PayrollSummary> getPayrollHistory(int months) {
        std::vector<PayrollSummary> result;

        int count = 0;
        for (auto it = g_payrollHistory.rbegin(); it != g_payrollHistory.rend() && count < months; ++it, ++count) {
            result.push_back(*it);
        }

        return result;
    }

    double getTotalPayrollCost(const std::string& startDate, const std::string& endDate) {
        double total = 0.0;

        for (const auto& summary : g_payrollHistory) {
            if (summary.period >= startDate && summary.period <= endDate) {
                total += summary.totalNetPay;
            }
        }

        return total;
    }

    // ============================================================================
    // PROJECT METRICS - Invoice/payment status and budget tracking
    // ============================================================================

    ProjectMetrics getProjectMetrics(const std::string& projectName) {
        ProjectMetrics metrics;
        metrics.projectName = projectName;

        // Calculate total expenses
        metrics.totalExpenses = getTotalExpensesByProject(projectName);

        // Get budget allocation
        if (g_projectBudgets.contains(projectName)) {
            metrics.budgetAllocated = g_projectBudgets[projectName];
        } else {
            metrics.budgetAllocated = 0.0;
        }

        // Calculate remaining budget
        metrics.budgetRemaining = metrics.budgetAllocated - metrics.totalExpenses;

        // Count transactions
        metrics.transactionCount = static_cast<int>(getExpensesByProject(projectName).size());

        return metrics;
    }

    std::vector<ProjectMetrics> getAllProjectMetrics() {
        std::vector<ProjectMetrics> result;
        std::vector<std::string> uniqueProjects;

        // Find unique project names
        for (const auto& expense : g_expenses) {
            if (std::find(uniqueProjects.begin(), uniqueProjects.end(), expense.projectName) == uniqueProjects.end()) {
                uniqueProjects.push_back(expense.projectName);
            }
        }

        // Pre-allocate capacity for performance
        result.reserve(uniqueProjects.size());

        // Get metrics for each project
        for (const auto& projectName : uniqueProjects) {
            result.push_back(getProjectMetrics(projectName));
        }

        return result;
    }

    bool updateProjectBudget(const std::string& projectName, double newBudget) {
        if (projectName.empty() || newBudget < 0.0) {
            return false;
        }

        g_projectBudgets[projectName] = newBudget;

        // Check thresholds after budget update
        checkThresholds();

        return true;
    }

    double calculateBudgetUtilization(const std::string& projectName) {
        if (!g_projectBudgets.contains(projectName) || g_projectBudgets[projectName] <= 0.0) {
            return 0.0;
        }

        double totalExpenses = getTotalExpensesByProject(projectName);
        double budget = g_projectBudgets[projectName];

        return (totalExpenses / budget) * 100.0;
    }

    // ============================================================================
    // ALERT THRESHOLDS - Breach detection and alerting
    // ============================================================================

    bool setAlertThreshold(const AlertThreshold& threshold) {
        if (threshold.metric.empty() || threshold.threshold <= 0.0) {
            return false;
        }

        // Remove existing threshold for the same metric
        g_alertThresholds.erase(
            std::remove_if(g_alertThresholds.begin(), g_alertThresholds.end(),
                [&threshold](const AlertThreshold& t) { return t.metric == threshold.metric; }),
            g_alertThresholds.end()
        );

        g_alertThresholds.push_back(threshold);
        return true;
    }

    std::vector<std::string> checkThresholds() {
        std::vector<std::string> alerts;

        for (const auto& threshold : g_alertThresholds) {
            // Check budget thresholds for each project
            if (threshold.metric == "budget_utilization") {
                auto allProjects = getAllProjectMetrics();
                for (const auto& project : allProjects) {
                    double utilization = calculateBudgetUtilization(project.projectName);

                    if (threshold.isPercentage && utilization >= threshold.threshold) {
                        std::ostringstream alert;
                        alert << "ALERT: Project '" << project.projectName
                              << "' has reached " << std::fixed << std::setprecision(1)
                              << utilization << "% budget utilization (threshold: "
                              << threshold.threshold << "%)";
                        alerts.push_back(alert.str());
                    }
                }
            }

            // Check payroll cost threshold
            if (threshold.metric == "payroll_cost" && !g_payrollHistory.empty()) {
                const auto& latest = g_payrollHistory.back();
                if (latest.totalNetPay >= threshold.threshold) {
                    std::ostringstream alert;
                    alert << "ALERT: Payroll cost for period '" << latest.period
                          << "' exceeds threshold: $" << std::fixed << std::setprecision(2)
                          << latest.totalNetPay << " (threshold: $" << threshold.threshold << ")";
                    alerts.push_back(alert.str());
                }
            }
        }

        return alerts;
    }

    bool isOverBudget(const std::string& projectName) {
        if (!g_projectBudgets.contains(projectName)) {
            return false;
        }

        double totalExpenses = getTotalExpensesByProject(projectName);
        double budget = g_projectBudgets[projectName];

        return totalExpenses > budget;
    }

    // ============================================================================
    // REPORTING - Generate formatted reports
    // ============================================================================

    std::string generateExpenseReport(const std::string& startDate, const std::string& endDate) {
        std::ostringstream report;

        report << "====================================\n";
        report << "      EXPENSE REPORT\n";
        report << "====================================\n";
        report << "Period: " << startDate << " to " << endDate << "\n";
        report << "------------------------------------\n\n";

        auto expenses = getExpensesByDateRange(startDate, endDate);

        if (expenses.empty()) {
            report << "No expenses found for this period.\n";
            return report.str();
        }

        // Group by project
        std::map<std::string, double> projectTotals;
        std::map<std::string, std::vector<ExpenseRecord>> projectExpenses;

        for (const auto& expense : expenses) {
            projectTotals[expense.projectName] += expense.amount;
            projectExpenses[expense.projectName].push_back(expense);
        }

        double grandTotal = 0.0;

        for (const auto& [projectName, total] : projectTotals) {
            report << "Project: " << projectName << "\n";
            report << "  Total: $" << std::fixed << std::setprecision(2) << total << "\n";
            report << "  Expenses:\n";

            for (const auto& expense : projectExpenses[projectName]) {
                report << "    - " << expense.date << " | " << expense.category
                       << " | $" << expense.amount;
                if (!expense.description.empty()) {
                    report << " | " << expense.description;
                }
                report << "\n";
            }

            report << "\n";
            grandTotal += total;
        }

        report << "------------------------------------\n";
        report << "GRAND TOTAL: $" << std::fixed << std::setprecision(2) << grandTotal << "\n";
        report << "====================================\n";

        return report.str();
    }

    std::string generatePayrollReport(const std::string& period) {
        std::ostringstream report;

        report << "====================================\n";
        report << "      PAYROLL REPORT\n";
        report << "====================================\n";
        report << "Period: " << period << "\n";
        report << "------------------------------------\n\n";

        auto summary = getPayrollSummaryForPeriod(period);

        if (summary.period.empty()) {
            report << "No payroll data found for this period.\n";
            return report.str();
        }

        report << "Employee Count:    " << summary.employeeCount << "\n";
        report << "Total Gross Pay:   $" << std::fixed << std::setprecision(2)
               << summary.totalGrossPay << "\n";
        report << "Total Deductions:  $" << summary.totalDeductions << "\n";
        report << "Total Net Pay:     $" << summary.totalNetPay << "\n\n";

        double avgGrossPay = summary.employeeCount > 0 ? summary.totalGrossPay / summary.employeeCount : 0.0;
        double avgNetPay = summary.employeeCount > 0 ? summary.totalNetPay / summary.employeeCount : 0.0;

        report << "Average Gross Pay: $" << avgGrossPay << "\n";
        report << "Average Net Pay:   $" << avgNetPay << "\n";
        report << "====================================\n";

        return report.str();
    }

    std::string generateProjectReport(const std::string& projectName) {
        std::ostringstream report;

        report << "====================================\n";
        report << "      PROJECT REPORT\n";
        report << "====================================\n";
        report << "Project: " << projectName << "\n";
        report << "------------------------------------\n\n";

        auto metrics = getProjectMetrics(projectName);

        report << "Budget Allocated:  $" << std::fixed << std::setprecision(2)
               << metrics.budgetAllocated << "\n";
        report << "Total Expenses:    $" << metrics.totalExpenses << "\n";
        report << "Budget Remaining:  $" << metrics.budgetRemaining << "\n";
        report << "Transaction Count: " << metrics.transactionCount << "\n\n";

        double utilization = calculateBudgetUtilization(projectName);
        report << "Budget Utilization: " << std::fixed << std::setprecision(1)
               << utilization << "%\n";

        if (isOverBudget(projectName)) {
            report << "\n*** WARNING: PROJECT IS OVER BUDGET ***\n";
        }

        report << "\n------------------------------------\n";
        report << "Recent Expenses:\n";
        report << "------------------------------------\n";

        auto expenses = getExpensesByProject(projectName);

        // Show last 10 expenses
        int count = 0;
        for (auto it = expenses.rbegin(); it != expenses.rend() && count < 10; ++it, ++count) {
            report << it->date << " | " << it->category << " | $"
                   << std::fixed << std::setprecision(2) << it->amount;
            if (!it->description.empty()) {
                report << " | " << it->description;
            }
            report << "\n";
        }

        report << "====================================\n";

        return report.str();
    }

    // ============================================================================
    // DATA EXPORT - Export monitoring data to files
    // ============================================================================

    bool exportExpensesToCSV(const std::string& filepath, const std::string& startDate, const std::string& endDate) {
        std::ofstream file(filepath);

        if (!file.is_open()) {
            return false;
        }

        // Write CSV header
        file << "Date,Project,Category,Amount,Description\n";

        auto expenses = getExpensesByDateRange(startDate, endDate);

        for (const auto& expense : expenses) {
            file << expense.date << ","
                 << expense.projectName << ","
                 << expense.category << ","
                 << std::fixed << std::setprecision(2) << expense.amount << ","
                 << expense.description << "\n";
        }

        file.close();
        return true;
    }

    bool exportPayrollToCSV(const std::string& filepath, const std::string& period) {
        std::ofstream file(filepath);

        if (!file.is_open()) {
            return false;
        }

        // Write CSV header
        file << "Period,Employee Count,Gross Pay,Deductions,Net Pay\n";

        auto summary = getPayrollSummaryForPeriod(period);

        if (!summary.period.empty()) {
            file << summary.period << ","
                 << summary.employeeCount << ","
                 << std::fixed << std::setprecision(2) << summary.totalGrossPay << ","
                 << summary.totalDeductions << ","
                 << summary.totalNetPay << "\n";
        }

        file.close();
        return true;
    }

    // ============================================================================
    // DATABASE INTEGRATION - Using db.h for persistence
    // ============================================================================

    // Initialize monitoring database
    bool initializeMonitoringDatabase(const std::string& dbName) {
        // Call db.h to create the database
        return createDatabase(dbName);
    }

    // Save expense to database using db.h
    bool saveExpenseToDatabase(const std::string& dbName, const ExpenseRecord& expense) {
        // Format expense data as a string
        std::ostringstream data;
        data << expense.projectName << "|"
             << expense.category << "|"
             << std::fixed << std::setprecision(2) << expense.amount << "|"
             << expense.date << "|"
             << expense.description;

        std::string expenseData = data.str();

        // Call db.h to append data
        return appendToDatabase(dbName, expenseData);
    }

    // Save payroll summary to database using db.h
    bool savePayrollToDatabase(const std::string& dbName, const PayrollSummary& summary) {
        // Format payroll data as a string
        std::ostringstream data;
        data << summary.period << "|"
             << summary.employeeCount << "|"
             << std::fixed << std::setprecision(2) << summary.totalGrossPay << "|"
             << summary.totalDeductions << "|"
             << summary.totalNetPay;

        std::string payrollData = data.str();

        // Call db.h to append data
        return appendToDatabase(dbName, payrollData);
    }

    // Save project budget to text file using db.h
    bool saveProjectBudgetToFile(const std::string& filename, const std::string& projectName, double budget) {
        // First, create the file if it doesn't exist
        if (!openDatabase(filename)) {
            createFileText(filename);
        }

        // Format budget data
        std::ostringstream data;
        data << projectName << "|" << std::fixed << std::setprecision(2) << budget;

        // Call db.h to append to file
        return appendFileText(filename, data.str(), true);
    }

    // Load project budget from text file using db.h
    double loadProjectBudgetFromFile(const std::string& filename, const std::string& projectName, int lineNumber) {
        // Call db.h to read file
        std::string line = readFileText(filename, lineNumber);

        if (line.empty()) {
            return 0.0;
        }

        // Parse the line (format: "ProjectName|Budget")
        size_t delimPos = line.find('|');
        if (delimPos != std::string::npos) {
            std::string name = line.substr(0, delimPos);
            if (name == projectName) {
                std::string budgetStr = line.substr(delimPos + 1);
                try {
                    return std::stod(budgetStr);
                } catch (...) {
                    return 0.0;
                }
            }
        }

        return 0.0;
    }

    // Example: Save all expenses to database
    bool saveAllExpensesToDatabase(const std::string& dbName) {
        // Initialize database first
        if (!initializeMonitoringDatabase(dbName)) {
            return false;
        }

        // Open database connection
        if (!openDatabase(dbName)) {
            return false;
        }

        // Save each expense
        for (const auto& expense : g_expenses) {
            if (!saveExpenseToDatabase(dbName, expense)) {
                closeDatabase(dbName);
                return false;
            }
        }

        // Close database connection
        return closeDatabase(dbName);
    }

} // namespace monitor
