# Stricter Validation Rules - Implementation Summary

## Overview

The input validation system has been updated with **stricter rules** to ensure data quality and consistency. The validation now enforces character type restrictions for names, positions, and numeric fields.

## Updated Validation Rules

### 1. **Name Fields** (Letters and Spaces Only)

**Affected Fields:**
- Employee Name
- Position
- Site Location
- Project Name
- Material Name

**Rules:**
- ✅ **Allowed**: Letters (A-Z, a-z) and spaces
- ❌ **Not Allowed**: Numbers, special characters, punctuation

**Examples:**
```
Valid:
  - "John Doe"
  - "Senior Manager"
  - "Main Site"
  - "Construction Project Alpha"
  - "Portland Cement"

Invalid:
  - "John123" (contains numbers)
  - "Manager-Senior" (contains hyphen)
  - "Site@Main" (contains @)
  - "Project#1" (contains #)
```

**Visual Indicator:**
- Red background on input field
- Error message: "Letters & spaces only"

---

### 2. **Numeric Fields** (Numbers and Decimal Point Only)

**Affected Fields:**
- Hourly Rate / Salary
- Hours Worked
- Advance
- Quantity
- Unit Price

**Rules:**
- ✅ **Allowed**: Digits (0-9) and decimal point (.)
- ❌ **Not Allowed**: Letters, spaces, special characters, currency symbols

**Examples:**
```
Valid:
  - "25"
  - "25.50"
  - "0.75"
  - "1000"
  - "40.5"

Invalid:
  - "25 pesos" (contains letters/spaces)
  - "$25" (contains currency symbol)
  - "25.50.00" (multiple decimal points)
  - "twenty-five" (contains letters)
  - "25,000" (contains comma)
```

**Visual Indicators:**
- Red background on input field
- Error messages:
  - "Numbers only, > 0" (for salary, unit price)
  - "Numbers only, 0-168" (for hours)
  - "Numbers only, >= 0" (for advance, quantity)

---

### 3. **Date Format** (ISO 8601 Standard)

**Affected Fields:**
- Start Date
- All date inputs

**Rules:**
- ✅ **Format**: YYYY-MM-DD (ISO 8601)
- ✅ **Validation**: Includes leap year checking
- ✅ **Range**: Year 1900-2100, valid month (01-12), valid day (01-31)

**Examples:**
```
Valid:
  - "2025-12-17"
  - "2024-02-29" (leap year)
  - "2025-01-01"
  - "2025-12-31"

Invalid:
  - "2025-13-01" (month > 12)
  - "2025-02-30" (invalid day for February)
  - "2025/12/17" (wrong separator)
  - "12-17-2025" (wrong order)
  - "2025-2-17" (missing leading zero)
  - "2025-02-29" (not a leap year)
```

**Leap Year Validation:**
- Correctly validates February 29th only in leap years
- Leap year rules: divisible by 4, except century years (must be divisible by 400)
- Examples:
  - 2024 is a leap year (Feb 29 valid)
  - 2025 is NOT a leap year (Feb 29 invalid)
  - 2000 was a leap year (divisible by 400)
  - 1900 was NOT a leap year (divisible by 100 but not 400)

**Visual Indicator:**
- Red background on input field
- Error message: "Format: YYYY-MM-DD"

---

## Technical Implementation

### Character Type Validation

```cpp
// Letters and spaces only (for names/positions)
auto isLettersAndSpacesOnly = [](const std::string& str) -> bool {
    if (str.empty()) return false;
    for (char c : str) {
        if (!std::isalpha(static_cast<unsigned char>(c)) && 
            !std::isspace(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    return true;
};

// Numbers and decimal point only
auto isValidDecimal = [](const std::string& str) -> bool {
    if (str.empty()) return false;
    bool hasDecimal = false;
    for (char c : str) {
        if (c == '.') {
            if (hasDecimal) return false; // No multiple decimals
            hasDecimal = true;
        } else if (!std::isdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    return true;
};
```

### ISO 8601 Date Validation with Leap Year

```cpp
auto isValidISODate = [](const std::string& str) -> bool {
    if (str.length() != 10) return false;
    if (str[4] != '-' || str[7] != '-') return false;
    
    // Validate format
    for (int i = 0; i < 10; ++i) {
        if (i == 4 || i == 7) continue;
        if (!std::isdigit(static_cast<unsigned char>(str[i]))) return false;
    }
    
    // Validate ranges
    const int year = std::stoi(str.substr(0, 4));
    const int month = std::stoi(str.substr(5, 2));
    const int day = std::stoi(str.substr(8, 2));
    
    if (year < 1900 || year > 2100) return false;
    if (month < 1 || month > 12) return false;
    
    // Days in month (with leap year support)
    const int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int maxDay = daysInMonth[month - 1];
    
    // Leap year check for February
    if (month == 2) {
        const bool isLeapYear = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        if (isLeapYear) maxDay = 29;
    }
    
    return day >= 1 && day <= maxDay;
};
```

## User Experience Impact

### Before (Lenient Validation)
- Could enter "John123" for name ❌
- Could enter "$25.50" for salary ❌
- Could enter "12/17/2025" for date ❌
- Could enter "Feb 29, 2025" ❌

### After (Strict Validation)
- Must enter "John Doe" for name ✅
- Must enter "25.50" for salary ✅
- Must enter "2025-12-17" for date ✅
- Cannot enter Feb 29 for non-leap years ✅

## Benefits

### Data Quality
- ✅ **Consistent Format**: All data follows strict format rules
- ✅ **No Junk Data**: Numbers can't appear in names, letters can't appear in prices
- ✅ **Valid Dates**: Impossible to enter invalid dates like Feb 30 or Feb 29 in non-leap years
- ✅ **Database Integrity**: Clean, standardized data in database

### User Experience
- ✅ **Clear Expectations**: Users know exactly what format is required
- ✅ **Immediate Feedback**: Invalid characters shown with red background instantly
- ✅ **Helpful Messages**: Specific error messages guide users
- ✅ **Prevents Mistakes**: Can't accidentally enter wrong type of data

### Developer Benefits
- ✅ **Easier Processing**: No need to clean/sanitize data
- ✅ **Predictable Data**: Always know what format data will be in
- ✅ **Reduced Bugs**: Type mismatches caught at input time
- ✅ **Better Reports**: Consistent formatting in generated reports

## Migration Notes

### Existing Data
If you have existing data that doesn't meet these strict rules:
1. Data already in database is **not affected**
2. Only **new entries** and **updates** must follow strict rules
3. Consider data cleanup script if needed for consistency

### Common User Adjustments

**Names with Numbers:**
- Old: "Site 123" → New: "Site One Two Three"
- Old: "Project#5" → New: "Project Five"

**Prices with Currency:**
- Old: "$25.50" or "25 PHP" → New: "25.50"
- Old: "1,000.00" → New: "1000.00"

**Dates:**
- Old: "12/17/2025" → New: "2025-12-17"
- Old: "Dec 17, 2025" → New: "2025-12-17"

## Testing Checklist

### Name Validation
- [ ] Try entering numbers in name field → Should show error
- [ ] Try entering special characters → Should show error
- [ ] Enter letters and spaces only → Should work
- [ ] Test with very long names (>100 chars) → Should show error

### Numeric Validation
- [ ] Try entering letters in salary field → Should show error
- [ ] Try entering currency symbols ($, ₱) → Should show error
- [ ] Try entering commas in large numbers → Should show error
- [ ] Enter only numbers and decimal point → Should work

### Date Validation
- [ ] Try "2025-13-01" → Should show error (invalid month)
- [ ] Try "2025-02-30" → Should show error (invalid day)
- [ ] Try "2025-02-29" → Should show error (not leap year)
- [ ] Try "2024-02-29" → Should work (is leap year)
- [ ] Try "2000-02-29" → Should work (divisible by 400)
- [ ] Try "1900-02-29" → Should show error (not divisible by 400)
- [ ] Try "2025/12/17" → Should show error (wrong separator)
- [ ] Try "12-17-2025" → Should show error (wrong order)
- [ ] Enter "2025-12-17" → Should work

## Configuration

All validation rules are centralized in:
- **Logic**: `src/handler/system.cpp` - `validateInput()` function
- **Types**: `src/handler/system.h` - `ValidationType` enum
- **UI**: `src/ui/ui.cpp` - Real-time validation and error messages

To modify rules, update the helper lambdas in `system.cpp`.

## Documentation

Updated documentation:
- ✅ `doc/INPUT_VALIDATION_SYSTEM.md` - Complete validation system reference
- ✅ `doc/VALIDATION_INDICATORS.md` - Visual indicator guide
- ✅ `src/handler/system.h` - API documentation in comments

