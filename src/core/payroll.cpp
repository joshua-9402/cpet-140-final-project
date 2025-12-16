/*
 * CpET 140 Final Project — Payroll module
 * StructuraCost - Core - Payroll module
 *
 * Contributors:
 *  Mark Clarence De Luna
 *  Jan Vincent Matanguihan
 *  Pol Justine Pailas
 *
 * Purpose
 * - Implements payroll logic: employee records, rates, time entries, deductions, taxes, payroll runs, and calculations.
 *
 * Boundaries
 * - No UI rendering and no DB or I/O operations.
 * - Persistence or queries must go through the db adapter (db.h) or other handler / service modules.
 * - Keep any platform-specific code out of this module (except for payroll related module).
 *
 * Notes
 * - Single-threaded by default; callers must synchronize if accessed from multiple threads or workers.
 * - Use integer/fixed-point types for money where possible; prefer RAII (Resource Acquisition Is Initialization) for
 *   resources and small, testable functions.
 * - Return explicit result types and avoid global mutable state.
 *
 *
 * For the tax computation:
 *
 * The New TRAIN (Tax Reform for Acceleration and Inclusion) Law (Republic Act No. 10963) affects payroll calculations in
 * the Philippines by revising income tax brackets and rates, which can impact the net pay of employees. Employers must
 * update their payroll systems to comply with the new tax structure, ensuring accurate tax withholding and reporting.
 *
 * TRAIN law income tax (annual taxable income, PHP) — table:
 * | Bracket (in PHP)           | Tax withholding formula (in PHP)                          |
 * |---------------------------:|:----------------------------------------------------------|
 * | up to 250000              | 0%                                                         |
 * | 250001–400000             | 20% of excess over 250000                                  |
 * | 400001–800000             | 30000 + 25% of excess over 400000                          |
 * | 800001–2000000            | 130000 + 30% of excess over 800000                         |
 * | 2000001–8000000           | 490000 + 32% of excess over 2000000                        |
 * | over 8000000              | 2410000 + 35% of excess over 8000000                       |
 */

#include <string>
#include "payroll.h"


// Gross Pay
double payroll::computeGross(const Employee& emp) {
    return emp.hourlyRate * emp.hoursWorked;
}


// Government Contributions
double payroll::computeSSS(const double gross) {
    return gross * 0.045; // 4.5% employee share
}


double payroll::computePhilHealth(const double gross) {
    return gross * 0.025; // 2.5% employee share
}


double payroll::computePagIbig(const double gross) {
    return std::min(gross * 0.02, 100.0); // 2%, capped at 100
}


// TRAIN Law tax (weekly pay)
double payroll::computeTax(const double weeklyGrossAfterDeductions) {
    const double annual = weeklyGrossAfterDeductions * 52; // annualized
    double tax = 0.0;

    if (annual <= 250000)
        tax = 0.0;
    else if (annual <= 400000)
        tax = (annual - 250000) * 0.20;
    else if (annual <= 800000)
        tax = 30000 + (annual - 400000) * 0.25;
    else if (annual <= 2000000)
        tax = 130000 + (annual - 800000) * 0.30;
    else if (annual <= 8000000)
        tax = 490000 + (annual - 2000000) * 0.32;
    else
        tax = 2410000 + (annual - 8000000) * 0.35;

    return tax / 52.0; // weekly tax
}// Compute everything
PayrollResult payroll::computePayroll(const Employee& emp) {
    PayrollResult result{};

    result.grossPay = computeGross(emp);
    result.sss = computeSSS(result.grossPay);
    result.philHealth = computePhilHealth(result.grossPay);
    result.pagIbig = computePagIbig(result.grossPay);

    // Taxable amount = gross - contributions
    const double taxableWeekly = result.grossPay - (result.sss + result.philHealth + result.pagIbig);
    result.tax = computeTax(taxableWeekly);

    // Net pay
    result.netPay = result.grossPay - (result.sss + result.philHealth + result.pagIbig + result.tax);

    return result;
}





