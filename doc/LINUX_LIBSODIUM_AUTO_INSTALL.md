# Linux Automatic libsodium Installation

## Overview

Linux packages (.deb and .rpm) for StructuraCost include an automatic libsodium installation system that runs during package installation. This ensures the application has all required dependencies without manual user intervention.

## How It Works

The `packaging/linux/postinst.sh` script runs automatically after package installation and follows this workflow:

### 1. Detection Phase
```
Check if libsodium is available:
├── System libraries (via ldconfig)
├── Bundled libraries (/opt/structuracost/lib)
└── If found → Skip installation
```

### 2. Package Manager Installation (Primary)
If libsodium is not found, attempt installation via system package manager:

| Distribution | Package Manager | Command |
|--------------|-----------------|---------|
| Debian/Ubuntu | apt-get | `apt-get install libsodium-dev libsodium23` |
| Fedora | dnf | `dnf install libsodium libsodium-devel` |
| RHEL/CentOS | yum | `yum install libsodium libsodium-devel` |
| openSUSE | zypper | `zypper install libsodium libsodium-devel` |
| Arch Linux | pacman | `pacman -S libsodium` |

**Advantages:**
- Fast (uses cached packages)
- Automatic updates via system package manager
- Shared with other applications

### 3. Source Build Installation (Fallback)
If package manager installation fails, build from source:

```bash
# Download libsodium 1.0.20 from official repository
wget https://download.libsodium.org/libsodium/releases/libsodium-1.0.20.tar.gz

# Extract and build
./configure --prefix=/opt/structuracost --libdir=/opt/structuracost/lib
make -j$(nproc)
make install

# Configure library path
echo "/opt/structuracost/lib" > /etc/ld.so.conf.d/structuracost.conf
ldconfig
```

**Advantages:**
- Works on any Linux distribution
- No dependency on package repositories
- Isolated installation (doesn't affect system)

**Requirements:**
- gcc
- make
- wget or curl

### 4. Failure Handling
If both methods fail, the script:
1. Displays clear error message with manual installation instructions
2. Exits with error code (prevents silent failures)
3. Shows platform-specific installation commands

## Testing the Script

### Test on Debian/Ubuntu:
```bash
# Simulate package installation
sudo /opt/structuracost/postinst.sh
```

### Test on RedHat/Fedora:
```bash
# Simulate package installation
sudo /opt/structuracost/postinst.sh
```

### Test source build (remove package manager first):
```bash
# Temporarily hide package manager
sudo mv /usr/bin/apt-get /usr/bin/apt-get.bak
sudo /opt/structuracost/postinst.sh
sudo mv /usr/bin/apt-get.bak /usr/bin/apt-get
```

## Integration with CMake/CPack

The postinst script is automatically included in packages via CMakeLists.txt:

```cmake
# Debian packages
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA 
    "${CMAKE_SOURCE_DIR}/packaging/linux/postinst.sh;")

# RPM packages  
set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE 
    "${CMAKE_SOURCE_DIR}/packaging/linux/postinst.sh")
```

### When packages are created:
- `.deb`: Script runs via DEBIAN/postinst
- `.rpm`: Script runs via %post scriptlet

## User Experience

### Successful Installation (Package Manager):
```
StructuraCost: Checking for libsodium...
Attempting to install libsodium from package manager...
Using apt-get...
✓ libsodium installed from package manager

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✓ StructuraCost installation complete!
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

To launch: structuracost
```

### Successful Installation (Source Build):
```
StructuraCost: Checking for libsodium...
libsodium not found - installing...
Package manager installation failed - building from source...
Building libsodium from source...
Downloading libsodium 1.0.20...
Extracting...
Configuring...
Building... (this may take a few minutes)
Installing to /opt/structuracost...
✓ libsodium built from source successfully

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✓ StructuraCost installation complete!
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

To launch: structuracost
```

### Installation with Existing Library:
```
StructuraCost: Checking for libsodium...
✓ libsodium found in system libraries
libsodium is already available - skipping installation

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✓ StructuraCost installation complete!
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

To launch: structuracost
```

## Security Considerations

1. **Official Sources**: Downloads only from official libsodium repository
2. **HTTPS**: Uses secure connections (wget/curl)
3. **Version Pinning**: Uses specific version (1.0.20) to ensure reproducibility
4. **Isolated Build**: Builds in `/tmp` with unique directory name (prevents conflicts)
5. **Cleanup**: Removes build artifacts after installation

## Maintenance

### Updating libsodium Version
Edit `packaging/linux/postinst.sh`:
```bash
LIBSODIUM_VERSION="1.0.21"  # Update version number
```

### Adding Support for New Package Managers
Add to `install_from_package_manager()`:
```bash
elif command -v <package-manager> &> /dev/null; then
    echo "Using <package-manager>..."
    <package-manager> install -y libsodium
    return $?
```

## Troubleshooting

### Issue: Script fails with "gcc not found"
**Solution**: Install build tools before package installation
```bash
# Debian/Ubuntu
sudo apt-get install build-essential

# Fedora/RHEL
sudo dnf groupinstall "Development Tools"
```

### Issue: Script fails to download
**Solution**: Check internet connection or proxy settings
```bash
# Test download manually
wget https://download.libsodium.org/libsodium/releases/libsodium-1.0.20.tar.gz
```

### Issue: Library not found after installation
**Solution**: Update library cache manually
```bash
sudo ldconfig
ldd /opt/structuracost/bin/structuracost
```

## Comparison: Linux vs Windows/macOS

| Feature | Linux | Windows | macOS |
|---------|-------|---------|-------|
| Prebuilt Required | No (auto-downloads) | Yes | Yes |
| Installation Method | Package manager or source | Bundled DLL | Bundled dylib |
| User Intervention | None | None | None |
| Disk Space | ~500KB | Bundled in installer | Bundled in .app |
| Build Time | 2-5 min (if building) | N/A | N/A |

## Benefits

1. **Zero User Friction**: Works immediately after package installation
2. **Flexible**: Adapts to system package availability
3. **Reliable**: Multiple fallback mechanisms
4. **Clean**: Uses system packages when possible
5. **Isolated**: Builds to app directory if needed
6. **Informative**: Clear progress messages and error reporting

