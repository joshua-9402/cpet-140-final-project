# Master Key Setup Guide

## Overview

This guide explains how to securely set up and manage the master encryption key for the Payroll and Monitoring System.

## First-Time Setup

### Step 1: Admin User Authentication

The first authenticated admin user will automatically trigger master key creation:

```cpp
// In auth.cpp - happens automatically during first login
bool authGateway(const std::string& username, const std::string& password, const std::string& database) {
    if (verifyAdminCredentials(username, password)) {
        // Generate secure random master key
        std::string masterKey = generateSecureRandomKey(32);
        
        // Encrypt and store it
        SecureStore::setupMasterKey(username, password, masterKey);
        
        return true;
    }
    return false;
}
```

### Step 2: Automatic Initialization

On subsequent logins, the system automatically retrieves the master key:

```cpp
// Automatic on login
SecureStore::initializeWithAuth("payroll.db", username, password);
```

## Security Flow

```
┌─────────────────┐
│  User Login     │
│  (username +    │
│   password)     │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Derive Key     │
│  SHA-256 Hash   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Decrypt        │
│  master.key     │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Initialize     │
│  Secure Store   │
└─────────────────┘
```

## File Permissions

### Linux/macOS
```bash
# Restrict access to master key file
chmod 600 master.key

# Only application owner can read/write
ls -la master.key
# Output: -rw------- 1 user group 256 Dec 20 10:30 master.key
```

### Windows
```powershell
# Remove inheritance and grant access only to current user
icacls master.key /inheritance:r
icacls master.key /grant:r "%USERNAME%:F"
```

## Backup Strategy

### What to Backup
1. `master.key` - Encrypted master key file
2. Admin credentials - Securely stored separately
3. Database files - Regular encrypted backups

### How to Backup
```bash
# Create encrypted backup
tar czf backup_$(date +%Y%m%d).tar.gz master.key *.db
openssl enc -aes-256-cbc -salt -in backup_*.tar.gz -out backup_*.tar.gz.enc
rm backup_*.tar.gz

# Store backup_*.tar.gz.enc in secure location
```

## Password Change Procedure

If an admin changes their password, re-encrypt the master key:

```cpp
void changeAdminPassword(const std::string& username, 
                        const std::string& oldPassword,
                        const std::string& newPassword) {
    // Retrieve master key with old credentials
    std::string masterKey = SecureStore::retrieveMasterKey(username, oldPassword);
    
    if (!masterKey.empty()) {
        // Re-encrypt with new credentials
        SecureStore::setupMasterKey(username, newPassword, masterKey);
        
        // Update password in authentication system
        updatePasswordInDatabase(username, newPassword);
    }
}
```

## Recovery Procedure

### If master.key is Lost
⚠️ **Data Loss**: Without the master key file, encrypted data cannot be recovered.

**Prevention:**
- Maintain secure backups of `master.key`
- Document recovery credentials in a secure vault
- Consider multiple authorized users

### If Admin Password is Forgotten
⚠️ **Access Denied**: Cannot decrypt master key without correct credentials.

**Options:**
1. Use password recovery mechanism (if implemented)
2. Use backup admin account
3. Restore from backup with known credentials

## Best Practices

### ✅ Do
- Store `master.key` in application directory with restricted permissions
- Backup `master.key` to secure offline storage
- Use strong admin passwords (12+ characters, mixed case, numbers, symbols)
- Log master key access attempts
- Implement account lockout after failed attempts

### ❌ Don't
- Commit `master.key` to version control
- Email or transmit `master.key` in plain text
- Store admin credentials in application code
- Share admin credentials across multiple users
- Store `master.key` in cloud storage without additional encryption

## Multi-User Support (Future Enhancement)

Current implementation supports single master key. For multi-user:

```cpp
// Store multiple encrypted copies of master key
struct MasterKeyEntry {
    std::string username;
    std::vector<uint8_t> encryptedMasterKey;
};

// Each authorized user gets their own encrypted copy
bool addAuthorizedUser(const std::string& newUsername, 
                       const std::string& newPassword,
                       const std::string& masterKey) {
    return SecureStore::setupMasterKey(newUsername, newPassword, masterKey);
}
```

## Troubleshooting

### Error: "Failed to retrieve master key"
**Cause:** Incorrect credentials or corrupted `master.key` file  
**Solution:** Verify credentials or restore from backup

### Error: "Master key file not found"
**Cause:** First-time setup or missing file  
**Solution:** System will auto-generate on first admin login

### Error: "Permission denied accessing master.key"
**Cause:** Incorrect file permissions  
**Solution:** 
```bash
chmod 600 master.key
chown $USER master.key
```

## Monitoring and Auditing

Add logging for security events:

```cpp
void logSecurityEvent(const std::string& event, const std::string& username) {
    std::ofstream log("security.log", std::ios::app);
    log << getCurrentTimestamp() << " | " 
        << event << " | " 
        << username << std::endl;
}

// Usage
logSecurityEvent("MASTER_KEY_ACCESS_SUCCESS", username);
logSecurityEvent("MASTER_KEY_ACCESS_FAILED", username);
```

## Support

For questions or issues with master key management:
- Review the main documentation in `secure_store.md`
- Check application logs in `security.log`
- Verify file permissions and backup status

## CI & Build Notes
- Ensure libsodium is available during CI configure step. See `doc/LIBSODIUM_SETUP.md`.
- The `systemSalt` used during build should reflect the release `version` tag if you want deterministic keys per release.

---

## Repository sync note
Last updated: 2025-11-29
