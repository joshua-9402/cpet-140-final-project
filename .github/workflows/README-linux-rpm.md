# Linux Build Workflows (Fedora/Red Hat Family)

## Overview

This workflow builds the payroll-and-monitoring-system for **Fedora/Red Hat family distributions** across **x64 and ARM64 architectures**.

## Supported Configurations

### Fedora/Red Hat Family Distributions
- **Fedora Latest** (Latest release, ~every 6 months)
- **Fedora 39** (Specific version)
- **Fedora 38** (Previous version)
- **AlmaLinux Latest** (RHEL-compatible, free)
- **AlmaLinux 9** (Based on RHEL 9)
- **AlmaLinux 8** (Based on RHEL 8)
- **Rocky Linux Latest** (RHEL-compatible, free)
- **Rocky Linux 9** (Based on RHEL 9)
- **Rocky Linux 8** (Based on RHEL 8)

### Derivative Distributions (Should also work)
These RPM-based derivatives should work with the builds:
- **Red Hat Enterprise Linux (RHEL)** - Commercial (use AlmaLinux/Rocky builds)
- **CentOS Stream** - Upstream of RHEL
- **Oracle Linux** - Based on RHEL
- **Nobara** - Gaming-focused Fedora derivative
- **Ultramarine Linux** - Fedora derivative
- **Scientific Linux** - Based on RHEL (discontinued but may still work)

### Architectures
- **x64** (AMD64/Intel 64-bit)
- **ARM64** (AArch64/ARM 64-bit)

### Total Combinations
**9 distributions × 2 architectures = 18 possible builds**

**Plus compatibility with numerous RPM-based derivatives!**

## Key Features

- ✅ **Manual trigger only** - Build when you need it
- ✅ **Multi-distribution support** - Fedora, AlmaLinux, Rocky Linux
- ✅ **Multi-architecture** - x64 and ARM64
- ✅ **Container-based builds** - Consistent environment
- ✅ **RPM ecosystem** - Uses dnf package manager
- ✅ **Configurable** - Choose Release or Debug
- ✅ **Optional LTO** - Enable/disable Link-Time Optimization
- ✅ **90-day retention** - Long artifact storage
- ✅ **RHEL-compatible** - AlmaLinux and Rocky Linux are RHEL clones

## How to Trigger a Build

### From GitHub Website

1. Go to your repository on GitHub
2. Click the **Actions** tab
3. Select **"Linux (Fedora/Red Hat)"** from the workflow list
4. Click the **"Run workflow"** button
5. Configure options:
   - **Distribution:** Choose your distribution
     - `fedora-latest` - Latest Fedora
     - `fedora-39` - Fedora 39
     - `fedora-38` - Fedora 38
     - `almalinux-latest` - Latest AlmaLinux
     - `almalinux-9` - AlmaLinux 9
     - `almalinux-8` - AlmaLinux 8
     - `rockylinux-latest` - Latest Rocky Linux
     - `rockylinux-9` - Rocky Linux 9
     - `rockylinux-8` - Rocky Linux 8
   - **Architecture:** Choose `x64` or `arm64`
   - **Build type:** Choose `Release` or `Debug`
   - **Enable LTO:** Check for maximum performance
6. Click **"Run workflow"**

### From GitHub CLI

```bash
# Fedora Latest x64 Release
gh workflow run "Linux (Fedora/Red Hat)" \
  --ref master \
  -f distribution=fedora-latest \
  -f architecture=x64 \
  -f build_type=Release \
  -f enable_lto=true

# AlmaLinux 9 ARM64 Release
gh workflow run "Linux (Fedora/Red Hat)" \
  --ref master \
  -f distribution=almalinux-9 \
  -f architecture=arm64 \
  -f build_type=Release \
  -f enable_lto=true

# Rocky Linux 9 x64 Debug
gh workflow run "Linux (Fedora/Red Hat)" \
  --ref master \
  -f distribution=rockylinux-9 \
  -f architecture=x64 \
  -f build_type=Debug \
  -f enable_lto=false
```

## Distribution Details

### Fedora (Bleeding Edge)

**Package Manager:** dnf  
**Compiler:** GCC (latest stable)  
**Best For:** Latest features, developers, Red Hat ecosystem testing

**Versions:**
- `fedora-latest` - Current Fedora release
- `fedora-39` - Fedora 39
- `fedora-38` - Fedora 38

**Release Cycle:** ~6 months  
**Support:** ~13 months per release

**Dependencies Installed:**
- gcc, gcc-c++, cmake, ninja-build
- X11/OpenGL development libraries
- FreeType and Fontconfig

### AlmaLinux (RHEL-Compatible, Free)

**Package Manager:** dnf  
**Compiler:** GCC (stable version matching RHEL)  
**Best For:** Enterprise, servers, RHEL compatibility without cost

**Versions:**
- `almalinux-latest` - Current AlmaLinux
- `almalinux-9` - AlmaLinux 9 (RHEL 9 compatible)
- `almalinux-8` - AlmaLinux 8 (RHEL 8 compatible)

**Release Cycle:** Follows RHEL (2-3 years)  
**Support:** 10 years per major version

**What is AlmaLinux?**
- 1:1 binary compatible with RHEL
- Free, community-supported
- CentOS replacement
- Perfect for servers and enterprise

### Rocky Linux (RHEL-Compatible, Free)

**Package Manager:** dnf  
**Compiler:** GCC (stable version matching RHEL)  
**Best For:** Enterprise, servers, RHEL compatibility, CentOS replacement

**Versions:**
- `rockylinux-latest` - Current Rocky Linux
- `rockylinux-9` - Rocky Linux 9 (RHEL 9 compatible)
- `rockylinux-8` - Rocky Linux 8 (RHEL 8 compatible)

**Release Cycle:** Follows RHEL (2-3 years)  
**Support:** 10 years per major version

**What is Rocky Linux?**
- 1:1 binary compatible with RHEL
- Free, community-supported
- Created by CentOS founder
- Enterprise-grade stability

### Why Fedora/Red Hat Family?

**Enterprise Support:**
- RHEL is the gold standard for enterprise Linux
- AlmaLinux/Rocky Linux provide free RHEL compatibility
- 10-year support lifecycle for enterprise distros

**Latest Features:**
- Fedora ships cutting-edge software
- Tests features for future RHEL releases
- Great for development

**Corporate Environments:**
- Many enterprises use RHEL
- AlmaLinux/Rocky Linux provide free alternatives
- SELinux enabled by default (security)

## Architecture Details

### x64 (AMD64/Intel 64-bit)

**Also known as:** amd64, x86_64, x86-64  
**Compatibility:** Intel and AMD processors  
**Runner:** Ubuntu latest (x64)  
**Most common:** Desktop and server systems

### ARM64 (AArch64)

**Also known as:** aarch64, ARM 64-bit  
**Compatibility:** ARM-based systems  
**Runner:** Ubuntu 22.04 ARM64  
**Common on:** ARM servers, cloud instances, some development boards

## Build Configuration

### Build Types

**Release** (Recommended for distribution)
- Maximum optimization (`-O3`)
- Link-Time Optimization (if enabled)
- No debug symbols
- Smaller binary size
- Fastest performance

**Debug** (For development)
- No optimization (`-O0`)
- Debug symbols (`-g`)
- Larger binary size
- Better debugging with GDB

### Link-Time Optimization (LTO)

**Enabled (Default):**
- Whole-program optimization
- Better performance (~5-10%)
- Longer build time
- Flags: `-O3 -flto -DNDEBUG`

**Disabled:**
- Standard optimization
- Faster build time
- Flags: `-O3 -DNDEBUG`

## What Gets Built

The workflow produces:
- Native Linux executable (ELF binary)
- Includes all assets (fonts, icons)
- Packaged as `.tar.gz` archive
- Distribution and architecture specific
- Dependencies: Dynamically linked (requires system libraries)

## Download Artifacts

After the workflow completes:

1. Go to the workflow run page
2. Scroll to **Artifacts** section
3. Download: `payroll-and-monitoring-system-fedora-latest-x64-Release.tar.gz`

**Retention:** 90 days

### Extract and Run

```bash
# Extract
tar -xzf payroll-and-monitoring-system-fedora-latest-x64-Release.tar.gz

# Run
./payroll-and-monitoring-system
```

### Check Dependencies

```bash
# See required libraries
ldd payroll-and-monitoring-system

# Common dependencies (same as Debian, different package names):
# - libfreetype.so.6
# - libGL.so.1
# - libX11.so.6
# - libstdc++.so.6
# - libgcc_s.so.1
```

## Compatibility Matrix

| Distribution | Architecture | Tested On | Likely Compatible With |
|--------------|--------------|-----------|------------------------|
| Fedora Latest | x64 | Fedora Latest | Fedora, CentOS Stream |
| Fedora Latest | arm64 | Fedora ARM64 | Fedora ARM, CentOS Stream ARM |
| Fedora 39 | x64 | Fedora 39 | Fedora 39+, Nobara |
| AlmaLinux 9 | x64 | AlmaLinux 9 | RHEL 9, Rocky 9, Oracle Linux 9, CentOS Stream 9 |
| AlmaLinux 9 | arm64 | AlmaLinux 9 ARM64 | RHEL 9 ARM, Rocky 9 ARM |
| AlmaLinux 8 | x64 | AlmaLinux 8 | RHEL 8, Rocky 8, Oracle Linux 8, CentOS 8 Stream |
| Rocky Linux 9 | x64 | Rocky Linux 9 | RHEL 9, AlmaLinux 9, Oracle Linux 9 |
| Rocky Linux 8 | x64 | Rocky Linux 8 | RHEL 8, AlmaLinux 8, Oracle Linux 8 |

## System Requirements (For Running)

**All Distributions:**
- OpenGL support (hardware or software)
- X11 display server
- Required shared libraries (installed via package manager)

**Installing Dependencies (End User):**

**Fedora/AlmaLinux/Rocky Linux:**
```bash
sudo dnf install freetype mesa-libGL libX11 libXrandr libXinerama libXcursor libXi
```

**RHEL (with subscription):**
```bash
sudo yum install freetype mesa-libGL libX11 libXrandr libXinerama libXcursor libXi
```

## Build Time

Typical build times:

| Configuration | x64 Time | ARM64 Time |
|--------------|----------|------------|
| Release + LTO | 8-12 min | 12-18 min |
| Release (no LTO) | 5-8 min | 8-12 min |
| Debug | 4-6 min | 6-10 min |

**Note:** ARM64 builds may use emulation if native ARM64 runners are unavailable.

## Distribution-Specific Notes

### Fedora

- **Latest features and packages**
- Short support cycle (~13 months)
- Great for developers and enthusiasts
- SELinux enabled by default
- Tests features for future RHEL

**Use when:**
- You want the latest software
- Developing for Red Hat ecosystem
- Desktop usage

### AlmaLinux

- **1:1 RHEL binary compatibility**
- Free and open source
- 10-year support lifecycle
- Enterprise-grade stability
- Community-governed

**Use when:**
- Need RHEL compatibility without cost
- Enterprise server deployments
- Long-term support required
- Migrating from CentOS

### Rocky Linux

- **1:1 RHEL binary compatibility**
- Free and open source
- 10-year support lifecycle
- Created by CentOS founder
- Enterprise-focused

**Use when:**
- Need RHEL compatibility without cost
- Enterprise server deployments
- CentOS replacement
- Prefer community-driven project

## Which Distribution to Choose?

**For end users (desktop):**
- Use Fedora Latest (cutting edge)

**For developers:**
- Use Fedora Latest (latest tools)

**For servers (production):**
- Use AlmaLinux 9 or Rocky Linux 9 (stable, long-term support)

**For RHEL compatibility:**
- Use AlmaLinux 9 or Rocky Linux 9 (free RHEL clones)

**For enterprise without RHEL subscription:**
- Use AlmaLinux 9 or Rocky Linux 9

## ARM64 Considerations

**Performance:**
- Native ARM64 build performs well on ARM hardware
- May use emulation if ARM64 runners unavailable

**Compatibility:**
- ARM servers (Oracle Cloud ARM, AWS Graviton with Fedora/AlmaLinux)
- Some development boards running Fedora ARM

**Limitations:**
- Fewer ARM64 runners available
- May use QEMU emulation (slower)
- Some packages may not be available

## Fedora vs AlmaLinux vs Rocky Linux

| Feature | Fedora | AlmaLinux | Rocky Linux |
|---------|--------|-----------|-------------|
| **Release Cycle** | 6 months | 2-3 years | 2-3 years |
| **Support** | 13 months | 10 years | 10 years |
| **RHEL Compatible** | No (upstream) | Yes (1:1) | Yes (1:1) |
| **Best For** | Desktop, latest features | Enterprise servers | Enterprise servers |
| **Stability** | Good | Excellent | Excellent |
| **Package Updates** | Frequent | Conservative | Conservative |
| **SELinux** | Yes | Yes | Yes |

## Best Practices

1. **Choose the right distribution:**
   - Fedora for latest features
   - AlmaLinux/Rocky for servers
   - Match your deployment target

2. **Test on target distribution:**
   - Don't assume cross-distribution compatibility
   - Verify dependencies are available

3. **Use enterprise distros for production:**
   - AlmaLinux 9 or Rocky Linux 9
   - Longer support period
   - More stable

4. **Include installation instructions:**
   - List required dependencies
   - Provide dnf install commands

## Performance Comparison

| Distribution | Build Speed | Binary Size | Optimization | Updates |
|--------------|-------------|-------------|--------------|---------|
| Fedora Latest | Fast | Medium | Excellent | Frequent |
| Fedora 39 | Fast | Medium | Excellent | Regular |
| AlmaLinux 9 | Fast | Medium | Good | Conservative |
| AlmaLinux 8 | Medium | Medium | Good | Conservative |
| Rocky Linux 9 | Fast | Medium | Good | Conservative |
| Rocky Linux 8 | Medium | Medium | Good | Conservative |

## Support

If you encounter issues:
1. Check the workflow run logs
2. Verify distribution and architecture
3. Check dependency installation logs
4. Test on actual target system

---

**Quick Start:** Actions → Linux (Fedora/Red Hat) → Select distribution + architecture → Run workflow

**Remember:** Test the binary on your actual target distribution and architecture!

