# Secure Store Module

## Overview

The `secure_store` module provides a simple yet practical solution for storing sensitive data such as hashed passwords, encryption keys, and user data in an encrypted SQLite database.

## Features

- **Password Hashing**: Uses SHA-256 with random salts to securely hash passwords
- **Data Encryption**: Uses XOR encryption with a master key for storing sensitive data
- **SQLite Backend**: Stores all data in a local SQLite database
- **Simple API**: Easy-to-use functions for storing and retrieving secure data

## Security Notes

⚠️ **Important**: This implementation uses XOR encryption which is suitable for educational purposes and basic obfuscation. For production systems, consider using:
- AES-256 encryption
- SQLCipher for database encryption
- Hardware security modules (HSM)
- Key derivation functions like PBKDF2 or Argon2

## API Reference

### Initialization

```cpp
bool initialize(const std::string& dbPath, const std::string& masterKey)
```

Initializes the secure store with a database path and master encryption key.

**Parameters:**
- `dbPath`: Path to the SQLite database file
- `masterKey`: Master key used for encrypting/decrypting data

**Returns:** `true` if initialization succeeds, `false` otherwise

**Example:**
```cpp
if (!SecureStore::initialize("secure.db", "my-master-key-123")) {
    // Handle initialization failure
}
```

### Password Management

```cpp
bool storeHashedPassword(const std::string& username, const std::string& password)
```

Stores a hashed password for a user. Automatically generates a random salt.

**Parameters:**
- `username`: Username identifier
- `password`: Plain-text password to hash and store

**Returns:** `true` if successful, `false` otherwise

**Example:**
```cpp
SecureStore::storeHashedPassword("john_doe", "SecurePass123!");
```

---

```cpp
bool verifyHashedPassword(const std::string& username, const std::string& password)
```

Verifies a password against the stored hash.

**Parameters:**
- `username`: Username identifier
- `password`: Plain-text password to verify

**Returns:** `true` if password matches, `false` otherwise

**Example:**
```cpp
if (SecureStore::verifyHashedPassword("john_doe", "SecurePass123!")) {
    // Password is correct
}
```

### Secure Data Storage

```cpp
bool storeSecureData(const std::string& key, const std::vector<uint8_t>& data)
```

Stores encrypted binary data.

**Parameters:**
- `key`: Unique identifier for the data
- `data`: Binary data to encrypt and store

**Returns:** `true` if successful, `false` otherwise

**Example:**
```cpp
std::vector<uint8_t> sensitiveData = {0x01, 0x02, 0x03};
SecureStore::storeSecureData("api_key", sensitiveData);
```

---

```cpp
std::vector<uint8_t> retrieveSecureData(const std::string& key)
```

Retrieves and decrypts stored data.

**Parameters:**
- `key`: Unique identifier for the data

**Returns:** Decrypted data as a byte vector, or empty vector if not found

**Example:**
```cpp
auto data = SecureStore::retrieveSecureData("api_key");
if (!data.empty()) {
    // Use decrypted data
}
```

### Cleanup

```cpp
void close()
```

Closes the secure store and clears sensitive data from memory.

**Example:**
```cpp
SecureStore::close();
```

## Usage Example

```cpp
#include "security/secure_store.h"
#include <iostream>

int main() {
    // Initialize
    if (!SecureStore::initialize("app_secure.db", "master-key-xyz")) {
        std::cerr << "Failed to initialize secure store\n";
        return 1;
    }
    
    // Store user password
    SecureStore::storeHashedPassword("alice", "MyPassword123");
    
    // Verify password
    if (SecureStore::verifyHashedPassword("alice", "MyPassword123")) {
        std::cout << "Login successful\n";
    }
    
    // Store sensitive data
    std::vector<uint8_t> apiKey = {0xDE, 0xAD, 0xBE, 0xEF};
    SecureStore::storeSecureData("api_key", apiKey);
    
    // Retrieve sensitive data
    auto retrieved = SecureStore::retrieveSecureData("api_key");
    
    // Cleanup
    SecureStore::close();
    
    return 0;
}
```

## Database Schema

### passwords table
| Column   | Type | Description                    |
|----------|------|--------------------------------|
| username | TEXT | Primary key, user identifier   |
| hash     | TEXT | SHA-256 hash of salted password|
| salt     | TEXT | Random 16-byte hex salt        |

### secure_data table
| Column | Type | Description                     |
|--------|------|---------------------------------|
| key    | TEXT | Primary key, data identifier    |
| data   | BLOB | Encrypted binary data           |

## Dependencies

- SQLite3
- `cryptography.h` (for SHA-256 hashing)
- Standard C++ library

## Best Practices

1. **Master Key Management**: Store the master key securely, never hardcode it
2. **Regular Password Updates**: Encourage users to update passwords periodically
3. **Secure Deletion**: When deleting sensitive data, overwrite before removal
4. **Access Control**: Limit which parts of your application can access the secure store
5. **Audit Logging**: Log access to sensitive data for security monitoring

## Future Improvements

- Implement AES-256-GCM encryption
- Add key rotation support
- Implement secure key derivation (PBKDF2/Argon2)
- Add audit trail for data access
- Support for hardware security modules

