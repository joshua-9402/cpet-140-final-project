/*
 * CpET 140 Final Project — Print/Export module header
 * StructuraCost - Handler - Print module
 *
 * Contributors:
 *  Joshua Literal
 *
 * Purpose
 * - Export payslips and project reports to HTML format
 * - Generate printable documents for payroll and project management
 *
 * Boundaries
 * - Interacts with database module to retrieve data
 * - Generates HTML files for browser viewing and printing
 *
 * Notes
 * - HTML output includes embedded styles and logos
 * - Supports both payslip and project report generation
 */

#ifndef CPET_140_FINAL_PROJECT_PRINT_H
#define CPET_140_FINAL_PROJECT_PRINT_H

#include <string>

// Export payslips to HTML file.
// outFile: path to output HTML file
// logoPath: path to logo image (will be embedded in HTML)
// Returns true on success, false on failure
bool exportPayslipsHtml(const std::string& outFile, const std::string& logoPath);

// Export project report to HTML file.
// projectId: the project ID (e.g., "PRJ-00001")
// outFile: path to output HTML file
// logoPath: path to logo image (will be embedded in HTML)
// Returns true on success, false on failure
bool exportProjectReportHtml(const std::string& projectId, const std::string& outFile, const std::string& logoPath);

#endif //CPET_140_FINAL_PROJECT_PRINT_H