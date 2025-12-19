/*
 * CpET 140 Final Project — Payroll module header
 * StructuraCost - Core - Payroll module
 *
 * Contributors:
 *  Joshua Literal
 *
 * Purpose
 * - Define payroll computation functions and data structures
 * - Handle employee wage calculations and deductions
 *
 * Boundaries
 * - Provides payroll calculation utilities to the UI and database modules
 *
 * Notes
 * - Implements computation for gross pay, SSS, PhilHealth, Pag-IBIG, and tax
 * - All calculations follow Philippine payroll standards
 */

#ifndef CPET_140_FINAL_PROJECT_INVENTORY_H
#define CPET_140_FINAL_PROJECT_INVENTORY_H


#include <string>
#include <algorithm>

struct Employee {
    int id;
    std::string name;
    double hourlyRate;
    double hoursWorked; // total hours in a week
    double regularHours{40.0}; // regular hours per week (used to compute overtime)
};

struct PayrollResult {
    double grossPay;
    double sss;
    double philHealth;
    double pagIbig;
    double tax;
    double netPay;
};

class payroll {
public:
    // Declare functions only
    static double computeGross(const Employee& emp);
    static double computeSSS(double gross);
    static double computePhilHealth(double gross);
    static double computePagIbig(double gross);
    static double computeTax(double weeklyGrossAfterDeductions);
    static PayrollResult computePayroll(const Employee& emp);

    // Simple helper to mirror current payslip math used by print.cpp
    // Gross = hourlyRate * hoursWorked; Net = Gross - (advance + others)
    // Government contributions and tax are not applied here (kept 0) to preserve existing behavior
    static PayrollResult computeSimpleFromHourly(double hourlyRate,
                                                 double hoursWorked,
                                                 double advance,
                                                 double others);
};


#endif //CPET_140_FINAL_PROJECT_INVENTORY_H