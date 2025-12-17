# Quick Reference: Automatic libsodium Installation (All Platforms)

## What It Does
Automatically downloads and installs libsodium during package installation on Linux, Windows, and macOS.

## How It Works

### Linux (.deb/.rpm)
```
Install package → postinst.sh runs → Checks libsodium → Installs if missing → Done
```

### Windows (NSIS Installer)
```
Run installer → Install files → PowerShell script runs → Checks libsodium → Downloads if missing → Done
```

### macOS (.pkg)
```
Install package → postinstall runs → Checks libsodium → Installs if missing → Done
```

## Installation Methods (by platform)

### Linux (in order of priority)
1. **Already Installed** → Skip installation
2. **Package Manager** → `apt-get/dnf/yum/zypper/pacman install libsodium`
3. **Build from Source** → Download, compile, install to `/opt/structuracost/lib`

### Windows (in order of priority)
1. **Bundled DLL** → Use pre-packaged libsodium.dll
2. **System PATH** → Check if already in Windows PATH
3. **Auto-Download** → Download official MSVC build and extract to install dir

### macOS (in order of priority)
1. **Bundled dylib** → Use pre-packaged library in app bundle
2. **Homebrew** → Install via `brew install libsodium`
3. **Build from Source** → Download, compile, install to app Frameworks

## User Experience

### Linux - With Package Manager (Fast):
```
Setting up structuracost...
StructuraCost: Checking for libsodium...
✓ libsodium installed from package manager
✓ StructuraCost installation complete!
```

### Linux - Building from Source (Slower):
```
Setting up structuracost...
StructuraCost: Checking for libsodium...
Building libsodium from source...
Downloading... Extracting... Configuring... Building... Installing...
✓ libsodium built from source successfully
✓ StructuraCost installation complete!
```

### Windows - Auto-Download:
```
Installing StructuraCost...
StructuraCost: Checking for libsodium...
libsodium not found - downloading...
Downloading libsodium 1.0.20 from official repository...
Extracting...
Installing to C:\Program Files\StructuraCost...
✓ libsodium installed successfully
✓ StructuraCost installation complete!
```

### macOS - Homebrew:
```
Installing StructuraCost...
StructuraCost: Checking for libsodium...
Attempting to install libsodium from Homebrew...
Using Homebrew...
✓ libsodium installed from Homebrew
✓ StructuraCost installation complete!
```

## Supported Platforms

### Linux Distributions:
- ✅ Debian / Ubuntu (apt-get)
- ✅ Fedora (dnf)
- ✅ RHEL / CentOS (yum)
- ✅ openSUSE (zypper)
- ✅ Arch Linux (pacman)
- ✅ Any Linux with gcc/make (source build)

### Windows:
- ✅ Windows 10/11 (x64)
- ✅ Windows Server 2016+ (x64)
- ⚠️ Requires PowerShell (pre-installed on modern Windows)

### macOS:
- ✅ macOS 11 Big Sur and later (Intel)
- ✅ macOS 11 Big Sur and later (Apple Silicon)
- ⚠️ Xcode Command Line Tools required for source build

## For Developers

### Test the scripts:

**Linux:**
```bash
sudo packaging/linux/postinst.sh
```

**Windows (PowerShell as Admin):**
```powershell
.\packaging\windows\install_libsodium.ps1 -InstallDir "C:\Program Files\StructuraCost"
```

**macOS:**
```bash
sudo packaging/macos/postinstall
```

### Update libsodium version:

- **Linux**: Edit `LIBSODIUM_VERSION` in `packaging/linux/postinst.sh`
- **Windows**: Edit `$version` in `packaging/windows/install_libsodium.ps1`
- **macOS**: Edit `LIBSODIUM_VERSION` in `packaging/macos/postinstall`

### Package creation:
```bash
cd build
cpack  # Creates packages for current platform with auto-install scripts
```

## Files

### Scripts:
- **Linux**: `packaging/linux/postinst.sh`
- **Windows**: `packaging/windows/install_libsodium.ps1`
- **macOS**: `packaging/macos/postinstall`

### Documentation:
- **Guide**: `doc/LINUX_LIBSODIUM_AUTO_INSTALL.md`
- **Summary**: `doc/LINUX_AUTO_INSTALL_SUMMARY.md`
- **Prebuilt Guide**: `doc/LIBSODIUM_PREBUILT_GUIDE.md`
- **This File**: `doc/LINUX_AUTO_INSTALL_QUICKREF.md`

## Comparison

| Platform | Method | User Action |
|----------|--------|-------------|
| Linux | Auto-install | None ✅ |
| Windows | Bundled DLL | None ✅ |
| macOS | Bundled dylib | None ✅ |

