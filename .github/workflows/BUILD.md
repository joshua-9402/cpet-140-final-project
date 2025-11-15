# Build Workflows Documentation

This document describes all the GitHub Actions workflows available for building the payroll-and-monitoring-system application.

## Table of Contents

- [Overview](#overview)
- [Automated Builds](#automated-builds)
- [Manual Linux Builds](#manual-linux-builds)
- [Manual macOS Builds](#manual-macos-builds)
  - [macOS ARM64 (Apple Silicon)](#macos-arm64-apple-silicon)
  - [macOS Intel (Big Sur and newer)](#macos-intel-big-sur-and-newer)
  - [macOS Legacy (Catalina and older)](#macos-legacy-catalina-and-older)
- [Manual Windows Builds](#manual-windows-builds)
  - [Windows x64](#windows-x64-manual)
  - [Windows ARM64](#windows-arm64-manual)
- [Workflow Comparison](#workflow-comparison)
- [How to Trigger Builds](#how-to-trigger-builds)
- [Downloading Artifacts](#downloading-artifacts)
- [Troubleshooting](#troubleshooting)

---

## Overview

This project provides multiple build workflows to support different platforms and architectures:

- **Automated builds** - Run automatically on push (Linux Ubuntu x64, Windows x64)
- **Manual Linux builds** - Run on-demand for Debian-based distributions (Ubuntu, Debian) and architectures (x64, ARM64)
- **Manual macOS builds** - Run on-demand for specific macOS targets (3 workflows)
- **Manual Windows builds** - Run on-demand for Windows x64 and ARM64

### Why Multiple macOS Workflows?

macOS has evolved significantly across versions and architectures:
- **Apple Silicon (ARM64)** vs **Intel (x86_64)** require different architectures
- **Newer macOS versions** support C++23 and modern features
- **Older macOS versions** need C++20 and careful API selection

Having separate workflows ensures optimal compatibility for each target.

---

## Automated Builds

### Multi-Platform Build
**File:** `build.yml`  
**Trigger:** Automatic on push to `master` branch (current repository)

**Platforms:**
- Ubuntu Linux (x64)
- Windows (x64) - Automated version

**Note:** macOS builds are manual-only to allow for architecture and OS version selection. Windows also has a manual workflow available for more control over build configuration.

**Features:**
- Runs automatically on push to `master`
- Builds for Linux and Windows simultaneously
- 30-day artifact retention
- Optimized Release builds with LTO

---

## Manual Linux Builds

**File:** `build-linux.yml`  
**Documentation:** `README-linux.md`

The manual Linux workflow allows you to build for **Debian-based Linux distributions** and **architectures**.

### Supported Distributions

**Official Distributions:**
- **Ubuntu 22.04** - Latest LTS (Jammy Jellyfish)
- **Ubuntu 20.04** - Previous LTS (Focal Fossa)
- **Ubuntu Latest** - Rolling latest
- **Debian 12** - Bookworm (Latest Stable)
- **Debian 11** - Bullseye (Oldstable)
- **Debian Latest** - Current Debian Stable

**Compatible Derivatives:**
- Linux Mint (based on Ubuntu)
- Pop!_OS (based on Ubuntu)
- elementary OS (based on Ubuntu)
- Zorin OS (based on Ubuntu)
- KDE neon (based on Ubuntu LTS)
- Raspberry Pi OS (based on Debian)
- MX Linux (based on Debian)
- And many more Debian/Ubuntu derivatives

### Supported Architectures

- **x64** (AMD64/x86_64) - Intel/AMD processors
- **ARM64** (aarch64) - ARM 64-bit processors (Raspberry Pi, ARM servers)

### Total Combinations

**6 distributions × 2 architectures = 12 possible builds**

**Plus compatibility with dozens of Debian/Ubuntu derivatives!**

### Configuration

- **Build Method:** Container-based (Docker)
- **Package Manager:** apt (Debian/Ubuntu standard)
- **C++ Standard:** C++23
- **Compiler:** GCC (version depends on distribution)
- **Build System:** Ninja
- **LTO:** Configurable (enabled by default)
- **Retention:** 90 days

### When to Use

- Building for specific Debian-based distribution
- Targeting ARM64 Linux devices (Raspberry Pi 4/5, ARM servers)
- Need reproducible builds across Debian/Ubuntu versions
- Testing compatibility with different Debian/Ubuntu versions
- Distributing to Linux Mint, Pop!_OS, or other derivatives

### Key Features

- ✅ **Debian-based distributions only** - Ubuntu, Debian, and derivatives
- ✅ **Multi-architecture** - x64 and ARM64
- ✅ **Container-based** - Consistent build environment
- ✅ **Configurable** - Release or Debug builds
- ✅ **Optional LTO** - Link-Time Optimization
- ✅ **90-day retention** - Long artifact storage
- ✅ **Wide compatibility** - Works on most Debian/Ubuntu derivatives

### How to Trigger

1. Go to Actions → **"Build Linux (Manual)"**
2. Click **"Run workflow"**
3. Select:
   - **Distribution** (Ubuntu 22.04, Debian 12, etc.)
   - **Architecture** (x64 or arm64)
   - **Build type** (Release or Debug)
   - **Enable LTO** (Yes or No)
4. Download the artifact when complete

### Artifact Names

```
payroll-and-monitoring-system-ubuntu-22.04-x64-Release.tar.gz
payroll-and-monitoring-system-debian-12-arm64-Release.tar.gz
payroll-and-monitoring-system-ubuntu-20.04-x64-Debug.tar.gz
```

### Use Cases

| Use Case | Recommended Build |
|----------|------------------|
| **General Linux users** | Ubuntu 22.04 x64 |
| **Linux Mint users** | Ubuntu 22.04 x64 (Mint 21) or Ubuntu 20.04 x64 (Mint 20) |
| **Pop!_OS users** | Ubuntu 22.04 x64 |
| **Servers** | Debian 12 x64 or Ubuntu 22.04 x64 |
| **Raspberry Pi 4/5** | Debian 12 arm64 or Ubuntu 22.04 arm64 |
| **ARM servers (AWS Graviton)** | Ubuntu 22.04 arm64 or Debian 12 arm64 |
| **Stable/conservative** | Debian 11 x64 |

---

## Manual Linux Builds (Fedora/Red Hat Family)

**File:** `build-linux-rpm.yml`  
**Documentation:** `README-linux-rpm.md`

The manual Linux RPM workflow allows you to build for **Fedora/Red Hat family distributions** and **architectures**.

### Supported Distributions

**Fedora (Bleeding Edge):**
- **Fedora Latest** - Current release
- **Fedora 39** - Specific version
- **Fedora 38** - Previous version

**AlmaLinux (RHEL-Compatible, Free):**
- **AlmaLinux Latest** - Current release
- **AlmaLinux 9** - Based on RHEL 9
- **AlmaLinux 8** - Based on RHEL 8

**Rocky Linux (RHEL-Compatible, Free):**
- **Rocky Linux Latest** - Current release
- **Rocky Linux 9** - Based on RHEL 9
- **Rocky Linux 8** - Based on RHEL 8

**Compatible Derivatives:**
- Red Hat Enterprise Linux (RHEL) - Use AlmaLinux/Rocky builds
- CentOS Stream
- Oracle Linux
- Nobara (Fedora-based)

### Supported Architectures

- **x64** (AMD64/x86_64) - Intel/AMD processors
- **ARM64** (aarch64) - ARM 64-bit processors

### Total Combinations

**9 distributions × 2 architectures = 18 possible builds**

**Plus compatibility with RHEL and other RPM-based derivatives!**

### Configuration

- **Build Method:** Container-based (Docker)
- **Package Manager:** dnf (Red Hat standard)
- **C++ Standard:** C++23
- **Compiler:** GCC (version depends on distribution)
- **Build System:** Ninja
- **LTO:** Configurable (enabled by default)
- **Retention:** 90 days

### When to Use

- Building for Fedora-based distributions
- Targeting RHEL/AlmaLinux/Rocky Linux servers
- Enterprise deployments requiring RHEL compatibility
- Need long-term support (AlmaLinux/Rocky: 10 years)
- Red Hat ecosystem development

### Key Features

- ✅ **Multi-distribution support** - Fedora, AlmaLinux, Rocky Linux
- ✅ **RHEL-compatible** - AlmaLinux and Rocky are 1:1 RHEL clones
- ✅ **Multi-architecture** - x64 and ARM64
- ✅ **Container-based** - Consistent build environment
- ✅ **Enterprise-grade** - 10-year support for AlmaLinux/Rocky
- ✅ **Configurable** - Release or Debug builds
- ✅ **Optional LTO** - Link-Time Optimization
- ✅ **90-day retention** - Long artifact storage

### How to Trigger

1. Go to Actions → **"Linux (Fedora/Red Hat)"**
2. Click **"Run workflow"**
3. Select:
   - **Distribution** (fedora-latest, almalinux-9, rockylinux-9, etc.)
   - **Architecture** (x64 or arm64)
   - **Build type** (Release or Debug)
   - **Enable LTO** (Yes or No)
4. Download the artifact when complete

### Artifact Names

```
payroll-and-monitoring-system-fedora-latest-x64-Release.tar.gz
payroll-and-monitoring-system-almalinux-9-arm64-Release.tar.gz
payroll-and-monitoring-system-rockylinux-9-x64-Debug.tar.gz
```

### Use Cases

| Use Case | Recommended Build |
|----------|------------------|
| **Fedora desktop users** | Fedora Latest x64 |
| **Enterprise servers** | AlmaLinux 9 x64 or Rocky Linux 9 x64 |
| **RHEL replacement** | AlmaLinux 9 x64 or Rocky Linux 9 x64 |
| **Latest features** | Fedora Latest x64 |
| **ARM servers** | AlmaLinux 9 arm64 or Rocky Linux 9 arm64 |
| **Long-term support** | AlmaLinux 9 x64 (10 years) or Rocky Linux 9 x64 (10 years) |

---

## Manual macOS Builds

All macOS workflows are **manual-trigger only** - they run when you explicitly trigger them from the GitHub Actions tab.

### macOS ARM64 (Apple Silicon)

**File:** `build-macos-arm64.yml`  
**Documentation:** `README-macos-arm64.md`

**Target Hardware:**
- Apple Silicon Macs (M1, M2, M3, M4)

**macOS Versions:**
- macOS Sonoma 14.0+

**Configuration:**
- **Runner:** `macos-14` (macOS Sonoma on ARM64)
- **Architecture:** ARM64 (native)
- **C++ Standard:** C++23
- **Deployment Target:** 14.0
- **LTO:** Enabled by default
- **Optimizations:** `-O3 -flto -DNDEBUG`

**When to Use:**
- You have an Apple Silicon Mac
- Running macOS Sonoma or newer
- Need maximum performance on modern hardware

**Artifact Name:**
```
payroll-and-monitoring-system-macos-arm64-Release.tar.gz
```

---

### macOS Intel (Big Sur and newer)

**File:** `build-macos-intel.yml`  
**Documentation:** `README-macos-intel.md`

**Target Hardware:**
- Intel Macs (Core i3/i5/i7/i9)

**macOS Versions:**
- macOS Big Sur 11.0 and newer
- Compatible with: Big Sur, Monterey, Ventura, Sonoma

**Configuration:**
- **Runner:** `macos-13` (macOS Ventura on Intel x86_64)
- **Architecture:** x86_64 (explicitly set)
- **C++ Standard:** C++23
- **Deployment Target:** 11.0
- **LTO:** Enabled by default
- **Optimizations:** `-O3 -flto -DNDEBUG`

**When to Use:**
- You have an Intel Mac
- Running macOS Big Sur 11.0 or newer
- Want modern C++23 features and optimizations

**Artifact Name:**
```
payroll-and-monitoring-system-macos-intel-Release.tar.gz
```

---

### macOS Legacy (Catalina and older)

**File:** `build-macos-legacy.yml`  
**Documentation:** `README-macos-legacy.md`

**Target Hardware:**
- Older Intel Macs (2012-2019 era)

**macOS Versions (Selectable):**
- macOS Catalina 10.15 (recommended)
- macOS Mojave 10.14
- macOS High Sierra 10.13

**Configuration:**
- **Runner:** `macos-13` (macOS Ventura on Intel x86_64)
- **Architecture:** x86_64 (explicitly set)
- **C++ Standard:** C++20 (for legacy compatibility)
- **Deployment Target:** 10.15, 10.14, or 10.13 (user selectable)
- **LTO:** Disabled by default (better compatibility)
- **Optimizations:** `-O3 -DNDEBUG` (no LTO)

**When to Use:**
- You have an Intel Mac running Catalina or older
- Supporting users on older macOS versions
- Need maximum backward compatibility

**Key Differences:**
- Uses **C++20** instead of C++23 (better support on old systems)
- **LTO disabled** to avoid linker issues on older macOS
- **Deployment target verification** with `otool`

**Artifact Name:**
```
payroll-and-monitoring-system-macos-legacy-10.15-Release.tar.gz
```

---

### Windows x64 (Manual)

**File:** `build-windows-x64.yml`  
**Documentation:** `README-windows-x64.md`

**Target Platform:**
- Windows 10/11 (64-bit)

**Configuration:**
- **Runner:** `windows-latest` (Windows Server 2022)
- **Architecture:** x64
- **C++ Standard:** C++23
- **Compiler:** MSVC (Microsoft Visual C++)
- **LTO (LTCG):** Enabled by default
- **Optimizations:** `/O2 /GL /LTCG`

**When to Use:**
- You want more control over Windows builds
- Need to test specific configurations
- Want longer artifact retention (90 days vs 30 days)
- Building for distribution

**Key Features:**
- Manual trigger only (build on demand)
- Configurable build type (Release/Debug)
- Optional LTCG (Link-Time Code Generation)
- Architecture verification with `dumpbin`
- 90-day artifact retention

**Artifact Name:**
```
payroll-and-monitoring-system-windows-x64-Release.zip
```

---

### Windows ARM64 (Manual)

**File:** `build-windows-arm64.yml`  
**Documentation:** `README-windows-arm64.md`

**Target Platform:**
- Windows 11/10 ARM64 (Snapdragon-based devices)

**Configuration:**
- **Runner:** `windows-latest` (Windows Server 2022 x64)
- **Build Architecture:** x64 (cross-compilation)
- **Target Architecture:** ARM64
- **C++ Standard:** C++23
- **Compiler:** MSVC ARM64 cross-compiler
- **LTO (LTCG):** Enabled by default
- **Optimizations:** `/O2 /GL /LTCG`

**When to Use:**
- Building for ARM64 Windows devices (Surface Pro X, etc.)
- Users have Snapdragon-based Windows PCs
- Want native ARM64 performance

**Key Features:**
- Manual trigger only (build on demand)
- Cross-compiled from x64 to ARM64
- Configurable build type (Release/Debug)
- Optional LTCG (Link-Time Code Generation)
- Architecture verification with `dumpbin`
- 90-day artifact retention

**Artifact Name:**
```
payroll-and-monitoring-system-windows-arm64-Release.zip
```

---

## Workflow Comparison

### Quick Reference

| Workflow         | Runner         | Target        | macOS Version | C++   | LTO  | Architecture |
|------------------|----------------|---------------|---------------|-------|------|--------------|
| **macOS ARM64**  | macos-14       | Apple Silicon | Sonoma 14.0+  | C++23 | ✅    | ARM64        |
| **macOS Intel**  | macos-13       | Modern Intel  | Big Sur 11.0+ | C++23 | ✅    | x86_64       |
| **macOS Legacy** | macos-13       | Old Intel     | Catalina      | C++20 | ✅    | x86_64       |
| **Windows x64**  | windows-latest | Windows 10/11 | N/A           | C++23 | ✅    | x64          |

### Architecture Decision Tree

```
What platform?
├─ Windows
│   ├─ Want manual control? → Use: Windows x64 (Manual) workflow
│   └─ Want automatic builds? → Use: Multi-Platform Build (automated)
│
├─ Linux → Use: Multi-Platform Build (automated)
│
└─ macOS → What hardware?
    ├─ Apple Silicon (M1/M2/M3)
    │   └─ Use: macOS ARM64 workflow
    │
    └─ Intel
        └─ What macOS version?
            ├─ Big Sur 11.0 or newer
            │   └─ Use: macOS Intel workflow
            │
            └─ Catalina 10.15 or older
                └─ Use: macOS Legacy workflow
                    └─ Select deployment target (10.15/10.14/10.13)
```

---

## How to Trigger Builds

### Automated Builds (Linux/Windows)

**Automatic:** Push to `main`, `master`, or `develop` branches
```bash
git push origin master
```

The workflow will automatically start building for Linux and Windows.

### Manual Builds (macOS/Windows/Linux)

#### From GitHub Website

1. Go to your repository on GitHub
2. Click the **Actions** tab
3. On the left sidebar, select the workflow you want:
   - **Build Linux (Manual)**
   - **Build macOS ARM64 (Manual)**
   - **Build macOS Intel (Manual)**
   - **Build macOS Legacy (Manual)**
   - **Build Windows x64 (Manual)**
   - **Build Windows ARM64 (Manual)**
4. Click the **"Run workflow"** button on the right
5. Configure the build options:
   - **Branch:** Select the branch to build (usually `master`)
   - **Distribution:** (Linux only) Choose Ubuntu, Fedora, Debian, or Arch
   - **Architecture:** (Linux only) Choose x64 or arm64
   - **Build type:** Choose `Release` or `Debug`
   - **Enable Link-Time Optimization:** Check/uncheck (default varies by workflow)
   - **Deployment target:** (macOS Legacy only) Choose 10.15, 10.14, or 10.13
6. Click the green **"Run workflow"** button to start

#### From GitHub CLI

**Linux (Ubuntu 22.04 x64):**
```bash
gh workflow run "Build Linux (Manual)" \
  --ref master \
  -f distribution=ubuntu-22.04 \
  -f architecture=x64 \
  -f build_type=Release \
  -f enable_lto=true
```

**Linux (Debian 12 ARM64 for Raspberry Pi):**
```bash
gh workflow run "Build Linux (Manual)" \
  --ref master \
  -f distribution=debian-12 \
  -f architecture=arm64 \
  -f build_type=Release \
  -f enable_lto=true
```

**macOS ARM64:**
```bash
gh workflow run "Build macOS ARM64 (Manual)" \
  --ref master \
  -f build_type=Release \
  -f enable_lto=true
```

**macOS Intel:**
```bash
gh workflow run "Build macOS Intel (Manual)" \
  --ref master \
  -f build_type=Release \
  -f enable_lto=true
```

**macOS Legacy (Catalina):**
```bash
gh workflow run "Build macOS Legacy (Manual)" \
  --ref master \
  -f build_type=Release \
  -f enable_lto=false \
  -f deployment_target=10.15
```

**Windows x64:**
```bash
gh workflow run "Build Windows x64 (Manual)" \
  --ref master \
  -f build_type=Release \
  -f enable_lto=true
```

---

## Downloading Artifacts

### From Automated Builds

1. Go to **Actions** tab
2. Click on the latest successful workflow run
3. Scroll to the **Artifacts** section at the bottom
4. Download the artifacts you need:
   - `payroll-and-monitoring-system-linux-x64.tar.gz`
   - `payroll-and-monitoring-system-windows-x64.zip`

**Retention:** 30 days

### From Manual macOS Builds

1. Go to **Actions** tab
2. Click on the workflow run you triggered
3. Wait for the build to complete (green checkmark)
4. Scroll to the **Artifacts** section at the bottom
5. Download the artifact:
   - ARM64: `payroll-and-monitoring-system-macos-arm64-Release.tar.gz`
   - Intel: `payroll-and-monitoring-system-macos-intel-Release.tar.gz`
   - Legacy: `payroll-and-monitoring-system-macos-legacy-10.15-Release.tar.gz`
   - Windows: `payroll-and-monitoring-system-windows-x64-Release.zip`

**Retention:** 90 days

### Extract and Run

**Linux/macOS:**
```bash
tar -xzf payroll-and-monitoring-system-*.tar.gz
open payroll-and-monitoring-system.app  # macOS
./payroll-and-monitoring-system          # Linux
```

**Windows:**
```powershell
Expand-Archive payroll-and-monitoring-system-windows-x64.zip
cd payroll-and-monitoring-system-windows-x64
.\payroll-and-monitoring-system.exe
```

---

## Troubleshooting

### Build Failed

1. Click on the failed workflow run
2. Click on the failed job (red X)
3. Click on the failed step
4. Read the error message

**Common Issues:**

**CMake Configuration Error:**
- Check `CMakeLists.txt` syntax
- Verify all source files are committed
- Ensure dependencies are correctly specified

**Compilation Error:**
- Check for C++ syntax errors in source code
- Verify C++ standard compatibility (C++20 vs C++23)
- Check for platform-specific code issues

**Linker Error:**
- LTO may be causing issues (try disabling it)
- Missing library dependencies
- Symbol conflicts

**Executable Not Found:**
- Build may have failed silently
- Check build verification step logs
- Look for earlier error messages

### macOS-Specific Issues

**Wrong Architecture:**
```bash
# Verify architecture
file payroll-and-monitoring-system.app/Contents/MacOS/payroll-and-monitoring-system
lipo -info payroll-and-monitoring-system.app/Contents/MacOS/payroll-and-monitoring-system
```

Should show:
- ARM64 workflow: `arm64`
- Intel/Legacy workflows: `x86_64`

**Wrong Deployment Target:**
```bash
# Check minimum macOS version
otool -l payroll-and-monitoring-system.app/Contents/MacOS/payroll-and-monitoring-system | grep -A 3 LC_VERSION_MIN_MACOSX
```

Should match the deployment target you selected.

**App Won't Run on Target macOS:**
- Verify you're using the correct workflow for your macOS version
- Check deployment target is correct
- Try right-click → Open (for Gatekeeper on first run)
- Check Console.app for crash logs

### Performance Issues

**Build Taking Too Long:**
- Reduce parallel jobs (change `-j 4` to `-j 2`)
- Disable LTO for faster builds
- Use Debug build type instead of Release

**Artifact Download Fails:**
- Artifacts expire (30 days for automated, 90 days for manual)
- Make sure build completed successfully
- Check your internet connection

### Linux/Windows Issues

**Linux: Missing Dependencies:**
- Workflow installs dependencies automatically
- Check if newer Ubuntu version needs different packages

**Windows: Executable in Wrong Location:**
- May be in `build/` or `build/Release/`
- Workflow checks both locations

---

## Build Configuration Details

### CMake Variables Used

**All Builds:**
- `CMAKE_BUILD_TYPE` - Release or Debug
- `CMAKE_CXX_STANDARD` - 23 (or 20 for legacy)
- `CMAKE_CXX_STANDARD_REQUIRED` - ON
- `CMAKE_CXX_EXTENSIONS` - OFF

**macOS-Specific:**
- `CMAKE_OSX_DEPLOYMENT_TARGET` - Minimum macOS version (11.0, 10.15, etc.)
- `CMAKE_OSX_ARCHITECTURES` - x86_64 or arm64 (for Intel builds only)

**Optimization Flags:**

**GCC/Clang (with LTO):**
```
-O3 -flto -DNDEBUG
```

**GCC/Clang (without LTO):**
```
-O3 -DNDEBUG
```

**MSVC:**
```
/O2 /GL /LTCG
```

### Dependencies Installed

**Linux (Ubuntu):**
- build-essential, cmake, ninja-build
- libxrandr-dev, libxinerama-dev, libxcursor-dev, libxi-dev
- libgl1-mesa-dev, libglu1-mesa-dev
- libfreetype6-dev, libfontconfig1-dev

**macOS:**
- cmake, ninja (via Homebrew)
- freetype (via Homebrew)
- Clang/LLVM (pre-installed)

**Windows:**
- ninja (via Chocolatey)
- MSVC (pre-installed)

---

## Best Practices

### For Development

1. **Use the workflow matching your development machine**
   - Faster feedback loop
   - Matches your local environment

2. **Test locally first**
   - Build locally before pushing
   - Saves GitHub Actions minutes

3. **Use Debug builds for development**
   - Faster compilation
   - Better debugging experience

### For Distribution

1. **Use Release builds**
   - Maximum optimization
   - Smaller binaries
   - Better performance

2. **Build for all target platforms**
   - Don't assume one build works everywhere
   - Test on actual hardware when possible

3. **For macOS, choose appropriate workflow**
   - ARM64 for Apple Silicon users
   - Intel for modern Intel Macs
   - Legacy for users on older macOS

### For Legacy macOS Support

1. **Choose oldest macOS you actually need**
   - Each older version adds testing burden
   - 10.15 (Catalina) is usually a good minimum

2. **Test on actual hardware**
   - VMs are helpful but not perfect
   - Real hardware catches edge cases

3. **Keep LTO disabled**
   - Unless you've tested it thoroughly
   - Compatibility > Performance for legacy

---

## Additional Resources

- **Workflow Files:** `.github/workflows/`
- **Detailed Docs:** 
  - `README-linux.md` (Debian/Ubuntu)
  - `README-linux-rpm.md` (Fedora/Red Hat)
  - `README-linux-suse.md` (SUSE Family)
  - `README-macos-arm64.md`
  - `README-macos-intel.md`
  - `README-macos-legacy.md`
  - `README-windows-x64.md`
  - `README-windows-arm64.md`
- **Main README:** `../../README.md`
- **CMakeLists.txt:** `../../CMakeLists.txt`

---

## Support

If you encounter issues:

1. Check this documentation
2. Read the specific workflow's README file
3. Look at workflow run logs in GitHub Actions
4. Check the main project README
5. File an issue with:
   - Workflow name
   - Error message
   - Build configuration used

---

## Summary

### Quick Start

**For Automated Builds (Linux/Windows):**
```bash
git push origin master
# Builds start automatically
```

**For Manual macOS Builds:**
1. Go to GitHub → Actions
2. Select appropriate macOS workflow
3. Click "Run workflow"
4. Choose configuration
5. Download artifact when complete

### Workflow Selection Guide

| You Have | You Run | Use This Workflow |
|----------|---------|-------------------|
| M1/M2/M3 Mac | Sonoma 14.0+ | **macOS ARM64** |
| Intel Mac | Big Sur 11.0+ | **macOS Intel** |
| Intel Mac | Catalina 10.15 | **macOS Legacy** (target 10.15) |
| Intel Mac | Mojave 10.14 | **macOS Legacy** (target 10.14) |
| Intel Mac | High Sierra 10.13 | **macOS Legacy** (target 10.13) |
| Linux x64 | Want auto builds | **Multi-Platform** (automatic) |
| Linux x64 | Want specific distro | **Linux (Manual)** - Choose distro |
| Linux ARM64 | Raspberry Pi, ARM server | **Linux (Manual)** - Choose distro + arm64 |
| Windows | Want auto builds | **Multi-Platform** (automatic) |
| Windows x64 | Want manual control | **Windows x64 (Manual)** |
| Windows ARM64 | Surface Pro X, Snapdragon | **Windows ARM64 (Manual)** |

---

**Last Updated:** November 15, 2025

