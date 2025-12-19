# Payroll & Related Formulas

This document collects all payroll-related formulas implemented in the project codebase, with short descriptions and references to the source files/locations where they are used.

> Note: All currency amounts are in Philippine Pesos (PHP). Hours are weekly hours unless noted otherwise.

---

## 1) Gross Pay
Definition: The basic gross pay for a pay period (weekly in this project) is computed from the hourly rate and total hours worked.

Formula:

    Gross = hourlyRate * hoursWorked

Location:
- src/core/payroll.cpp — payroll::computeGross(const Employee&)
- src/handler/print.cpp — used when generating payslip HTML: `const double gross = data.salary * data.hoursWorked;`

Explanation: Multiplies the employee's hourly rate by the total hours worked in the week to obtain gross earnings before deductions.

---

## 2) SSS (Social Security System) Contribution
Definition: Employee share of SSS contribution.

Formula:

    SSS = Gross * 0.045  (4.5% of gross)

Location:
- src/core/payroll.cpp — payroll::computeSSS(const double)

---

## 3) PhilHealth Contribution
Definition: Employee share of PhilHealth contribution.

Formula:

    PhilHealth = Gross * 0.025  (2.5% of gross)

Location:
- src/core/payroll.cpp — payroll::computePhilHealth(const double)

---

## 4) Pag-IBIG Contribution
Definition: Employee share of Pag-IBIG contribution, with an upper cap.

Formula:

    PagIbig = min(Gross * 0.02, 100.0)  (2% of gross, capped at PHP 100)

Location:
- src/core/payroll.cpp — payroll::computePagIbig(const double)

---

## 4.1) Deductions breakdown (displayed on exported payslip)
Definition: The exported payslip now shows a detailed breakdown of all deductions so the employee can see exactly what has been withheld.

Items shown and how they are computed or taken from data:

- Advance
  - Source: `EMPLOYEES.ADVANCE` in payroll DB / `employee.advance` in code.
  - Shown as a direct deduction amount (no further computation).

- Others
  - Source: `employee.others` (optional additional deductions recorded in DB).
  - Shown directly as a deduction amount.

- SSS, PhilHealth (HMO), Pag-IBIG
  - Computed from gross pay using the formulas in sections 2, 3, and 4 above.
  - SSS = Gross * 0.045
  - PhilHealth = Gross * 0.025
  - Pag-IBIG = min(Gross * 0.02, 100.0)

- Tax
  - Computed using the TRAIN law logic (annualize taxable weekly amount, apply bracketed tax, then divide by 52). See section 5 for the full bracket logic.

Total deductions used on the payslip:

    TotalDeductions = Advance + Others + SSS + PhilHealth + PagIbig + Tax

Net pay displayed on the payslip:

    NetPay = Gross - TotalDeductions

Location (where the payslip is produced/updated):
- src/handler/print.cpp — `makePayslipHtml` now computes and includes a detailed deductions table and updates the printed Net Pay to subtract all deductions.
- Computation of SSS/PhilHealth/Pag-IBIG/Tax uses functions in `src/core/payroll.cpp`.

Notes:
- The pay export code uses attendance-derived weekly hours (from `WEEKLY_ATTENDANCE`) to compute `hoursWorked` and therefore gross pay; the `Advance` and `Others` fields come from the payroll DB.

---

## 5) Tax (TRAIN law) — weekly withholding
Definition: Computes weekly tax withholding by annualizing the taxable weekly income, applying the TRAIN tax brackets, then converting the annual tax back to weekly.

Steps / Formula:
1. Annualize taxable weekly amount:

       annual = weeklyTaxableAmount * 52

2. Apply TRAIN tax bracket calculation (annual):

       if annual <= 250000:
           annualTax = 0
       else if annual <= 400000:
           annualTax = (annual - 250000) * 0.20
       else if annual <= 800000:
           annualTax = 30000 + (annual - 400000) * 0.25
       else if annual <= 2000000:
           annualTax = 130000 + (annual - 800000) * 0.30
       else if annual <= 8000000:
           annualTax = 490000 + (annual - 2000000) * 0.32
       else:
           annualTax = 2410000 + (annual - 8000000) * 0.35

3. Convert back to weekly tax:

       weeklyTax = annualTax / 52

Location:
- src/core/payroll.cpp — payroll::computeTax(const double weeklyGrossAfterDeductions)

Notes:
- The function parameter `weeklyGrossAfterDeductions` is expected to be the weekly gross pay after subtracting government contributions (SSS, PhilHealth, Pag-IBIG).

---

## 6) Taxable Weekly Amount (used before calling tax computation)
Definition: The weekly amount considered for tax computation after subtracting employee government contributions.

Formula:

    taxableWeekly = grossPay - (SSS + PhilHealth + PagIbig)

Location:
- src/core/payroll.cpp — in payroll::computePayroll

---

## 7) Net Pay (detailed payroll compute)
Definition: Net pay after subtracting government contributions and tax from gross pay.

Formula:

    NetPay = Gross - (SSS + PhilHealth + PagIbig + Tax)

Location:
- src/core/payroll.cpp — payroll::computePayroll

---

## 8) Simple payslip math (legacy — printing/export behaviour updated)
Definition: Historically, `print.cpp` used a simplified payslip total where government contributions and tax were not applied when generating the exported HTML payslip. The exported payslip has been updated to show detailed deductions (see section 4.1).

Legacy formula (still referenced in comments / simple helper):

    Gross = ratePerHour * hoursWorked
    Total = Gross - (Advance + Others)

Location:
- src/handler/print.cpp — legacy usage previously computed `const double total = gross - data.advance - data.others;` but `makePayslipHtml` is now updated to include detailed deductions.
- src/core/payroll.h — comment for `computeSimpleFromHourly`

Notes:
- The export function still uses attendance-derived `hoursWorked` as input; the legacy simple total is preserved only for backward compatibility in a helper and comments.

---

## 9) Weekly Hours Sum (attendance)
Definition: Total weekly hours computed by summing daily attendance entries (Sunday through Saturday).

Formula:

    hoursWorked = Sun + Mon + Tue + Wed + Thu + Fri + Sat

Location:
- src/handler/print.cpp — when fetching WEEKLY_ATTENDANCE and computing `emp.hoursWorked`

---

## References / Source files
- src/core/payroll.cpp
- src/core/payroll.h
- src/handler/print.cpp (updated to include deductions in exported payslip)
- src/ui/ui.cpp (labeling and display, not computation)

---

## 10) Overtime computation (implemented)
Definition: Overtime pay applies to hours worked beyond an employee's `regularHours` per week (default 40.0 hours). Overtime hours are paid at an overtime multiplier (1.5x by default).

Formulas:

    overtimeHours = max(0, hoursWorked - regularHours)
    overtimeRate = hourlyRate * overtimeMultiplier   // overtimeMultiplier = 1.5 by default
    overtimePay = overtimeHours * overtimeRate

Gross pay (with overtime):

    basePay = min(hoursWorked, regularHours) * hourlyRate
    gross = basePay + overtimePay

Location:
- Implementation: `src/core/payroll.cpp` — `payroll::computeGross` now includes overtime using `Employee::regularHours` and a 1.5x multiplier.
- Display: `src/handler/print.cpp` — `makePayslipHtml` computes `overtimeHours` and `overtimePay` and displays them on the payslip when overtime applies.

Notes:
- `Employee::regularHours` default is 40.0 hours per week; you can change this value per-employee in code if needed before calling payroll computations.
- The overtime multiplier is currently fixed at 1.5 in code; if you need different multipliers (night differential, holiday, double-time), we can add more rules and parameters.
- The payslip will only show the overtime rows when `overtimeHours > 0`.
