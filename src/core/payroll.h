#ifndef CPET_140_FINAL_PROJECT_INVENTORY_H
#define CPET_140_FINAL_PROJECT_INVENTORY_H


#include <string>
#include <algorithm>

struct Employee {
    int id;
    std::string name;
    double hourlyRate;
    double hoursWorked; // total hours in a week
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
};


#endif //CPET_140_FINAL_PROJECT_INVENTORY_H