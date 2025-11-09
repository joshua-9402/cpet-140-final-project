/*
 * CpET 140 Final Project — Payroll module
 *
 * Contributors:
 *  Mark Clarence De Luna
 *  Jan Vincent Matanguihan
 *  Pol Justine Pailas
 *
 * Purpose
 * - Implements payroll logic: employee records, rates, time entries,
 *   deductions, taxes, payroll runs, and deterministic calculations.
 *
 * Boundaries
 * - No UI rendering and no raw SQLite calls here. Persistence must be
 *   performed via the db adapter or other narrow persistence adapters.
 * - Keep IO and platform-specific code out of this module.
 *
 * Notes
 * - Single-threaded by default; callers must synchronize if accessed from
 *   multiple threads or workers.
 * - Use integer/fixed-point types for money where possible; prefer RAII for
 *   resources and small, testable functions. Return explicit result types
 *   and avoid global mutable state.
 *
 *
 * For the tax computation:
 *
 * The New TRAIN Law (Republic Act No. 10963) affects payroll calculations in the Philippines by revising income tax
 * brackets and rates, which can impact the net pay of employees. Employers must update their payroll systems to comply
 * with the new tax structure, ensuring accurate tax withholding and reporting.
 *
 * TRAIN law income tax (annual taxable income, PHP) — table:
 * | Bracket                    | Tax withholding formula                                   |
 * |---------------------------:|:----------------------------------------------------------|
 * | up to 250000              | 0%                                                         |
 * | 250001–400000             | 20% of excess over 250000                                  |
 * | 400001–800000             | 30000 + 25% of excess over 400000                          |
 * | 800001–2000000            | 130000 + 30% of excess over 800000                         |
 * | 2000001–8000000           | 490000 + 32% of excess over 2000000                        |
 * | over 8000000              | 2410000 + 35% of excess over 8000000                       |
 */

#include "payroll.h"