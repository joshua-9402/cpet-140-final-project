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

## 8) Simple payslip math (used by printing/export)
Definition: A simplified payslip total used by `print.cpp` when preparing HTML payslips. This mirrors legacy behavior where government contributions and tax were not applied in the exported payslip.

Formula:

    Gross = ratePerHour * hoursWorked
    Total = Gross - (Advance + Others)

Location:
- src/handler/print.cpp — `const double gross = data.salary * data.hoursWorked;` and `const double total = gross - data.advance - data.others;`
- src/core/payroll.h — comment for `computeSimpleFromHourly`

Notes:
- `print.cpp` explicitly documents that attendance-derived hours are used to compute `hoursWorked`.

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
- src/handler/print.cpp
- src/ui/ui.cpp (labeling and display, not computation)

---

If you'd like, I can:
- Add a short worked example (numeric) for each formula.
- Export this `doc/FORMULAS.md` into the repo (I can create the file if you want — it's currently prepared but not yet written to disk).
- Include equations in LaTeX form or add cross-links to exact line numbers.

Tell me which extras you want and I'll apply them.
