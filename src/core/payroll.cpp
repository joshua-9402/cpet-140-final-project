/*
 * CpET 140 Final Project — Payroll module
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

#include "payroll.h"