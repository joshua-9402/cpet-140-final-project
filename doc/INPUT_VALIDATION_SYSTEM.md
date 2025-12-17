# Input Validation System

## Overview

The input validation system is a centralized validation framework implemented in `system.cpp` that ensures all user inputs are properly validated before being processed or stored in the database. This system provides a single, consistent interface for validating various types of inputs throughout the application.

## Location

- **Header File**: `src/handler/system.h`
- **Implementation**: `src/handler/system.cpp`
- **Usage**: Throughout `src/ui/ui.cpp`

## How It Works

The validation system uses a single function with two parameters:

```cpp
std::string system::validateInput(ValidationType validationType, const std::string& input);
```

### Parameters

1. **validationType** - An enum specifying what kind of validation to perform
2. **input** - The string to be validated

### Return Value

- **On Success**: Returns the input string (passed validation)
- **On Failure**: Returns an empty string (failed validation)

This design makes it easy to check validation results:

```cpp
std::string validatedInput = system::validateInput(system::ValidationType::NAME, userInput);
if (validatedInput.empty()) {
    // Validation failed
} else {
    // Validation passed, use validatedInput
}
```

## Validation Types

The system supports the following validation types:

### Basic Validators

| Type | Description | Example |
|------|-------------|---------|
| `NOT_EMPTY` | Checks if input is not empty or whitespace | "John" ✓, "" ✗ |
| `DIGITS_ONLY` | Checks if input contains only digits | "12345" ✓, "123a" ✗ |
| `ALPHANUMERIC_SPACES` | Allows alphanumeric characters and spaces | "Room 123" ✓, "Room@123" ✗ |

### Numeric Validators

| Type | Description | Range |
|------|-------------|-------|
| `POSITIVE_INTEGER` | Positive whole numbers | > 0 |
| `NON_NEGATIVE_INTEGER` | Zero or positive whole numbers | >= 0 |
| `POSITIVE_DECIMAL` | Positive decimal numbers | > 0.0 |
| `NON_NEGATIVE_DECIMAL` | Zero or positive decimal numbers | >= 0.0 |

### Format Validators

| Type | Description | Format Example |
|------|-------------|---------------|
| `PROJECT_ID_FORMAT` | Project ID format | PRJ-00001 |
| `DATE_FORMAT` | **ISO 8601 date format** with leap year validation | 2025-12-17 |

### Application-Specific Validators

| Type | Description | Constraints |
|------|-------------|-------------|
| `EMPLOYEE_ID` | Employee ID validation | Positive integer |
| `NAME` | Name validation | **Letters and spaces only**, max 100 chars |
| `POSITION` | Position validation | **Letters and spaces only**, max 50 chars |
| `SALARY` | Salary validation | **Numbers and decimal point only**, > 0 |
| `HOURS` | Hours worked validation | **Numbers and decimal point only**, 0-168 |
| `ADVANCE` | Advance payment validation | **Numbers and decimal point only**, >= 0, can be empty |
| `QUANTITY` | Quantity validation | **Numbers and decimal point only**, >= 0 |
| `MATERIAL_ID` | Material ID validation | Not empty |

## Implementation Examples

### Employee Management - Add Employee

```cpp
// Validate all inputs before adding
const std::string validatedName = system::validateInput(system::ValidationType::NAME, nameStr);
const std::string validatedPosition = system::validateInput(system::ValidationType::POSITION, positionStr);
const std::string validatedLocation = system::validateInput(system::ValidationType::NAME, locationStr);
const std::string validatedSalary = system::validateInput(system::ValidationType::SALARY, salaryStr);
const std::string validatedHours = system::validateInput(system::ValidationType::HOURS, hoursWorkedStr);
const std::string validatedAdvance = system::validateInput(system::ValidationType::ADVANCE, advanceStr);

if (validatedName.empty()) {
    system::logMessage(system::messageClassification::WARNING, 
        "DB: Name is required and must be valid (max 100 characters).\n");
} else if (validatedPosition.empty()) {
    system::logMessage(system::messageClassification::WARNING, 
        "DB: Position is required and must be valid (max 50 characters).\n");
} else if (validatedSalary.empty()) {
    system::logMessage(system::messageClassification::WARNING, 
        "DB: Hourly rate must be a positive number.\n");
} else {
    // All validation passed, proceed with database operation
    monitor::addEmployee(validatedName, validatedPosition, validatedLocation, 
                        validatedSalary, validatedHours, validatedAdvance);
}
```

### Project Management - Update Project

```cpp
// Validate project ID (required for update)
const std::string validatedProjectID = system::validateInput(
    system::ValidationType::PROJECT_ID_FORMAT, projectIDStr);

if (validatedProjectID.empty()) {
    system::logMessage(system::messageClassification::WARNING, 
        "DB: Valid Project ID (PRJ-#####) is required for update.\n");
} else {
    // Validate optional fields only if provided
    std::string validatedProjectName = projectNameStr.empty() ? projectNameStr : 
        system::validateInput(system::ValidationType::NAME, projectNameStr);
    std::string validatedStartDate = startDateStr.empty() ? startDateStr : 
        system::validateInput(system::ValidationType::DATE_FORMAT, startDateStr);
    
    // Proceed with update using validated data
    db::updateDatabase(dbPath, validatedProjectID, setClause);
}
```

### Materials Management - Add Material

```cpp
const std::string validatedProjectID = system::validateInput(
    system::ValidationType::PROJECT_ID_FORMAT, materialProjectIDStr);
const std::string validatedMaterialID = system::validateInput(
    system::ValidationType::MATERIAL_ID, materialIDStr);
const std::string validatedMaterialName = system::validateInput(
    system::ValidationType::NAME, materialNameStr);
const std::string validatedQuantity = system::validateInput(
    system::ValidationType::QUANTITY, materialQuantityStr);
const std::string validatedUnitPrice = system::validateInput(
    system::ValidationType::SALARY, materialUnitPriceStr);

// Check all validations and provide specific error messages
if (validatedProjectID.empty()) {
    system::logMessage(system::messageClassification::WARNING, 
        "DB: Valid Project ID (PRJ-#####) is required.\n");
} else if (validatedQuantity.empty()) {
    system::logMessage(system::messageClassification::WARNING, 
        "DB: Quantity must be a non-negative number.\n");
} else {
    // All validated, add to database
    db::appendDatabase(expenseDbPath, materialData);
}
```

## Validation Logic Details

### Project ID Format (PRJ-#####)
- Must start with "PRJ-"
- Followed by at least 5 digits
- Example: PRJ-00001, PRJ-12345

### Date Format (YYYY-MM-DD)
- Exactly 10 characters
- Format: YYYY-MM-DD
- Year range: 1900-2100
- Month range: 01-12
- Day range: 01-31

### Hours Worked
- Must be a non-negative decimal number
- Maximum: 168 (hours in a week)
- Can include decimals for partial hours

### Advance Payment
- Special case: Empty input is valid (defaults to 0)
- If provided, must be a non-negative decimal

## Benefits

1. **Centralized Validation**: All validation logic is in one place, making it easier to maintain and update
2. **Consistent Interface**: Same function signature for all validation types
3. **Clear Error Handling**: Returns empty string on failure, making it easy to detect validation failures
4. **Type Safety**: Enum-based validation types prevent typos and provide IDE autocomplete
5. **Logging Integration**: Validation failures are logged with descriptive messages
6. **Flexible**: Easy to add new validation types as needed
7. **Performance**: Efficient validation with minimal overhead

## Adding New Validation Types

To add a new validation type:

1. Add the enum value to `ValidationType` in `system.h`:
```cpp
enum class ValidationType {
    // ...existing types...
    NEW_TYPE,  // Add your new type
};
```

2. Add the validation logic in `system.cpp`:
```cpp
case ValidationType::NEW_TYPE:
    // Add your validation logic here
    if (/* validation condition */) {
        return input;
    }
    return "";
```

3. Use it in your code:
```cpp
std::string validated = system::validateInput(system::ValidationType::NEW_TYPE, userInput);
```

## Best Practices

1. **Always validate user input** before database operations
2. **Provide specific error messages** to help users understand what went wrong
3. **Log validation failures** for debugging and audit purposes
4. **Check return value** - empty string means validation failed
5. **Use appropriate validation type** - don't use SALARY for NAME validation
6. **Handle empty inputs** appropriately (some fields allow empty, others don't)

## Testing

To test the validation system:

1. Try entering invalid data (empty strings, wrong formats, negative numbers where positive expected)
2. Check the log files in `logs/` directory for validation error messages
3. Verify that invalid data is rejected before reaching the database
4. Confirm that valid data passes through correctly

## Future Enhancements

Potential improvements to consider:

- Add validation for email addresses
- Add validation for phone numbers
- Add validation for postal codes
- Add minimum/maximum value validators
- Add regex-based custom validators
- Add multi-field cross-validation support

