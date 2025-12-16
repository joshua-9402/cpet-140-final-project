# Print Handler Module Documentation

## Overview
The Print Handler module is responsible for generating HTML-based reports that can be viewed in a browser and printed. It currently supports two types of reports:
1. **Payslip Reports** - Employee payroll information
2. **Project Reports** - Project cost analysis with materials breakdown

## Features
- Generates print-ready HTML with embedded styling
- Embeds company logo as base64 data URI for offline viewing
- Automatically opens generated reports in the default browser
- Cross-platform support (Windows, macOS, Linux)
- Professional design matching the company's visual identity (accent color: #d88c28)

## API Reference

### `exportPayslipsHtml`
Exports employee payslips to an HTML file.

**Signature:**
```cpp
bool exportPayslipsHtml(const std::string& outFile, const std::string& logoPath);
```

**Parameters:**
- `outFile`: Path to the output HTML file (absolute or relative)
- `logoPath`: Path to the company logo image (PNG, JPG, or GIF)

**Returns:** `true` on success, `false` on failure

**Example Usage:**
```cpp
const std::string outputPath = appConfig::g_dataDirectory + "payslips_output.html";
const std::string logoPath = HelloImGui::AssetFileFullPath("icons/business_logo.png");

if (exportPayslipsHtml(outputPath, logoPath)) {
    std::cout << "Payslips exported successfully!\n";
} else {
    std::cerr << "Failed to export payslips.\n";
}
```

**Notes:**
- Fetches all employees from the payroll database automatically
- Generates paginated output (12 payslips per page)
- Page size: 8.5in × 13in with 2×6 grid layout

---

### `exportProjectReportHtml`
Exports a comprehensive project report to an HTML file.

**Signature:**
```cpp
bool exportProjectReportHtml(const std::string& projectId, const std::string& outFile, const std::string& logoPath);
```

**Parameters:**
- `projectId`: The project ID (e.g., "PRJ-00001")
- `outFile`: Path to the output HTML file (absolute or relative)
- `logoPath`: Path to the company logo image (PNG, JPG, or GIF)

**Returns:** `true` on success, `false` on failure

**Example Usage:**
```cpp
const std::string projectId = "PRJ-00001";
const std::string outputPath = appConfig::g_dataDirectory + "project_report_" + projectId + ".html";
const std::string logoPath = HelloImGui::AssetFileFullPath("icons/business_logo.png");

if (exportProjectReportHtml(projectId, outputPath, logoPath)) {
    std::cout << "Project report exported successfully!\n";
} else {
    std::cerr << "Failed to export project report.\n";
}
```

**Notes:**
- Fetches project information from `base_project.db`
- Fetches materials from the project-specific expense database (e.g., `expense/PRJ-00001.db`)
- Displays project details: ID, name, status, start date, notes
- Lists all materials with quantities, unit prices, and total costs
- Calculates and displays total material cost
- Page size: 8.5in × 11in (standard letter size)

---

## System Interface

The print module is accessed through the `system` class for higher-level operations:

### `system::printPayslips`
```cpp
static bool printPayslips(const std::string& dbPath, const std::string& logoPath, const std::vector<int>& employeeIds);
```

Wrapper function that generates payslips and saves them to the data directory.

**Example:**
```cpp
if (system::printPayslips("", logoPath, {})) {
    system::logMessage(system::messageClassification::INFO, "Payslips exported successfully.\n");
}
```

### `system::printProjectReport`
```cpp
static bool printProjectReport(const std::string& projectId, const std::string& logoPath);
```

Wrapper function that generates a project report and saves it to the data directory.

**Example:**
```cpp
const std::string projectId = "PRJ-00001";
const std::string logoPath = HelloImGui::AssetFileFullPath("icons/business_logo.png");

if (system::printProjectReport(projectId, logoPath)) {
    system::logMessage(system::messageClassification::INFO, "Project report exported successfully.\n");
}
```

---

## Report Formats

### Payslip Layout
- **Grid:** 2 columns × 6 rows per page (12 payslips per page)
- **Paper Size:** 8.5in × 13in (bond paper)
- **Margins:** 0.3in top/bottom, 0.25in left/right
- **Border:** 2px solid accent color (#d88c28)
- **Font Size:** 7-12px (optimized for compact printing)

**Content Sections:**
1. Header with logo and "OFFICIAL PAYSLIP" title
2. Employee information (ID, name, position, location)
3. Earnings and deductions table
4. Net pay summary (highlighted in accent color)
5. Signature lines for employee and authorizer

### Project Report Layout
- **Paper Size:** 8.5in × 13in (bond paper)
- **Margins:** 0.5in all sides
- **Border:** 2px solid accent color (#d88c28)
- **Font Size:** 12-24px (readable for screen and print)

**Content Sections:**
1. Header with logo and "PROJECT REPORT" title
2. Project information grid (ID, name, status, start date, notes)
3. Materials & supplies table with:
   - Material ID
   - Material name
   - Quantity
   - Unit price (₱)
   - Total cost (₱)
4. Summary box with total material cost
5. Footer with generation timestamp

---

## Database Dependencies

### Payslip Report
- **Database:** `data/payroll/base_payroll.db`
- **Table:** `EMPLOYEES`
- **Columns:** EMPLOYEE_ID, NAME, POSITION, SITE_LOCATION, SALARY, HOURS_WORK, ADVANCE

### Project Report
- **Main Database:** `data/projects/base_project.db`
  - **Table:** `PROJECT_LIST`
  - **Columns:** PROJECT_ID, PROJECT_NAME, STATUS, START_DATE, NOTE

- **Expense Database:** `data/projects/expense/{PROJECT_ID}.db` (e.g., `PRJ-00001.db`)
  - **Table:** `MATERIALS`
  - **Columns:** MATERIAL_ID, MATERIAL_NAME, QUANTITY, UNIT_PRICE

---

## Design Guidelines

Both reports follow the company's design system:
- **Primary Accent Color:** #d88c28 (orange/gold)
- **Background Highlights:** #f9f3e8 (light beige)
- **Typography:** Arial sans-serif
- **Color Accuracy:** Uses `print-color-adjust: exact` for faithful printing

---

## Error Handling

The module provides clear error messages for common issues:
- Database not found
- No data available
- Invalid project ID
- File write failures
- Logo image not found

All errors are logged to stderr with descriptive messages.

---

## Platform Support

The module automatically detects the platform and uses the appropriate command to open the generated HTML:

- **Windows:** `start`
- **macOS:** `open`
- **Linux:** `xdg-open`

---

## Future Enhancements

Potential improvements for future versions:
- Filter payslips by specific employee IDs
- Support for date range selection
- Include labor costs in project reports
- PDF export option
- Email distribution
- Custom templates
- Multi-language support

---

## Testing

To test the project report feature:

1. Run the application in test mode
2. Navigate to the "Project Report Printing Test" section
3. Enter a valid project ID (e.g., "PRJ-00001")
4. Click "Print Project Report"
5. The report will open automatically in your default browser

Ensure test data exists in:
- `data/projects/base_project.db` (project list)
- `data/projects/expense/PRJ-00001.db` (materials for the project)
