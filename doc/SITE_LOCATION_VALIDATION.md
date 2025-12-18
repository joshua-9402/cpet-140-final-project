# Site Location Validation Enhancement

## Overview
Added a new `SITE_LOCATION` validation type to the validation system that properly validates site location entries including project IDs in the PRJ-XXXXX format.

## Change Summary
**Date:** December 18, 2024

### What Changed
Previously, site location used the `NAME` validation type, which only accepted letters and spaces. This prevented project IDs (like "PRJ-00001") from being validated correctly.

**Before:**
- Validation Type: `ValidationType::NAME`
- Allowed: Letters and spaces only
- Issue: Project IDs (PRJ-XXXXX) failed validation

**After:**
- Validation Type: `ValidationType::SITE_LOCATION`
- Allowed: 
  - "Main Office"
  - "Warehouse"
  - "PRJ-XXXXX" (where X is any digit, minimum 5 digits)

## Technical Implementation

### Files Modified

#### 1. `src/handler/system.h`
Added new validation type to the enum:
```cpp
enum class ValidationType {
    // ...existing types...
    MATERIAL_ID,            // Validate material ID (not empty)
    SITE_LOCATION           // Validate site location (Main Office, Warehouse, or PRJ-XXXXX)
};
```

#### 2. `src/handler/system.cpp`
Implemented validation logic:
```cpp
case ValidationType::SITE_LOCATION:
    if (!isNotEmpty(input)) return "";
    // Accept "Main Office", "Warehouse", or "PRJ-XXXXX" format
    if (input == "Main Office" || input == "Warehouse") {
        return input;
    }
    // Check if it matches PRJ-XXXXX format
    if (isValidProjectID(input)) {
        return input;
    }
    return "";
```

#### 3. `src/ui/ui.cpp`
Updated all site location validations to use the new type:
- Real-time validation (line ~809)
- Add Employee validation (line ~920)
- Update Employee validation (line ~958)

**Changed from:**
```cpp
system::validateInput(system::ValidationType::NAME, locationStr)
```

**Changed to:**
```cpp
system::validateInput(system::ValidationType::SITE_LOCATION, locationStr)
```

## Validation Rules

### Accepted Values

1. **"Main Office"** - Exact match (case-sensitive)
2. **"Warehouse"** - Exact match (case-sensitive)
3. **Project IDs** - Format: `PRJ-XXXXX` where:
   - Must start with "PRJ-"
   - Followed by at least 5 digits (can be more)
   - Examples:
     - ✅ `PRJ-00001`
     - ✅ `PRJ-12345`
     - ✅ `PRJ-000001` (6 digits also valid)
     - ❌ `PRJ-123` (too few digits)
     - ❌ `PRJ-ABC12` (contains letters)
     - ❌ `prj-00001` (wrong case)

### Validation Logic

The validation uses the existing `isValidProjectID` lambda function:
```cpp
auto isValidProjectID = [](const std::string& str) -> bool {
    if (str.length() < 9) return false; // "PRJ-" + at least 5 digits
    if (str.substr(0, 4) != "PRJ-") return false;
    for (size_t i = 4; i < str.length(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(str[i]))) return false;
    }
    return true;
};
```

## Benefits

1. **Proper Validation**: Project IDs are now correctly validated
2. **Consistency**: All site locations follow the same validation rules
3. **User Feedback**: Real-time validation shows users if their input is valid
4. **Data Integrity**: Prevents invalid site location entries in the database
5. **Extensibility**: Easy to add more fixed location options in the future

## Impact on Existing Features

### Compatible Features
- ✅ Site Location Dropdown - Works seamlessly
- ✅ Project Payroll Tracking - Properly validates project assignments
- ✅ Employee Add/Update/Delete - All operations validate correctly
- ✅ Load Employee Data - Loaded locations pass validation

### No Breaking Changes
- Existing "Main Office" and "Warehouse" entries remain valid
- Existing project assignments (PRJ-XXXXX) now properly validate
- No database migration needed
- Backward compatible with all existing data

## Testing

### Test Cases Verified

1. **Fixed Locations**
   - ✅ "Main Office" - Valid
   - ✅ "Warehouse" - Valid
   - ❌ "main office" - Invalid (case-sensitive)
   - ❌ "Office" - Invalid (not in allowed list)

2. **Project IDs**
   - ✅ "PRJ-00001" - Valid
   - ✅ "PRJ-99999" - Valid
   - ✅ "PRJ-000001" - Valid (6 digits)
   - ❌ "PRJ-001" - Invalid (too few digits)
   - ❌ "PRJ-ABCDE" - Invalid (letters in number part)
   - ❌ "prj-00001" - Invalid (wrong case)

3. **Edge Cases**
   - ❌ "" (empty) - Invalid
   - ❌ "   " (whitespace) - Invalid
   - ❌ "PRJ-" - Invalid (no digits)
   - ❌ "00001" - Invalid (missing prefix)

## Usage Examples

### In Code
```cpp
// Validate a site location
std::string location = "PRJ-00001";
std::string validated = system::validateInput(
    system::ValidationType::SITE_LOCATION, 
    location
);

if (!validated.empty()) {
    // Location is valid, use it
    monitor::updateEmployee(empId, name, position, validated, ...);
} else {
    // Location is invalid, show error
    system::logMessage(system::messageClassification::WARNING, 
        "Invalid site location\n");
}
```

### User Workflow
1. User selects or types site location
2. Real-time validation highlights field red if invalid
3. Validation messages appear in logs if needed
4. Only valid locations can be saved to database

## Future Enhancements

Potential improvements:
- Add more fixed locations (e.g., "Remote", "Client Site")
- Support for custom location validation rules
- Case-insensitive matching for fixed locations
- Location aliases (e.g., "Main" → "Main Office")

## See Also
- `doc/SITE_LOCATION_DROPDOWN_AND_PROJECT_PAYROLL.md` - Site location dropdown feature
- `doc/INPUT_VALIDATION_SYSTEM.md` - General validation system documentation
- `doc/STRICT_VALIDATION_RULES.md` - Validation rules overview

