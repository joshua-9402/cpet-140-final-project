# Windows ARM64 Build Workflow

## Overview

This workflow builds the payroll-and-monitoring-system specifically for **Windows ARM64** (ARM-based Windows devices).

## Key Features

- ✅ **Manual trigger only** - Runs only when you explicitly trigger it
- ✅ **Windows 11 ARM64 compatible** - Works on ARM-based Windows devices
- ✅ **ARM64 architecture** - Native ARM64 executables
- ✅ **Cross-compiled** - Built using MSVC ARM64 cross-compiler on x64 runner
- ✅ **Configurable** - Choose Release or Debug build
- ✅ **Optional LTO** - Enable/disable Link-Time Code Generation (LTCG)
- ✅ **90-day retention** - Artifacts kept for 3 months
- ✅ **Architecture verification** - Confirms ARM64 build with `dumpbin`
- ✅ **MSVC compiler** - Microsoft Visual C++ ARM64 cross-compiler

## Compatibility

This build works on:
- ✅ **Windows 11 ARM64** - Native performance (Surface Pro X, etc.)
- ✅ **Windows 10 ARM64** - Native performance
- ✅ **Windows 11/10 x64** - Via emulation (slower, not recommended)
- ❌ **Windows 32-bit** - Not supported

## Target Devices

**Native ARM64 Windows devices:**
- Microsoft Surface Pro X
- Microsoft Surface Pro 9 (5G)
- Lenovo ThinkPad X13s
- Samsung Galaxy Book Go
- Other Snapdragon-based Windows devices

## How to Trigger a Build

### From GitHub Website

1. Go to your repository on GitHub
2. Click the **Actions** tab
3. Select **"Build Windows ARM64 (Manual)"** from the workflow list (left side)
4. Click the **"Run workflow"** button (right side)
5. Configure options:
   - **Branch:** Choose the branch to build (usually `master` or `main`)
   - **Build type:** Choose `Release` (optimized) or `Debug` (with debug symbols)
   - **Enable Link-Time Optimization:** Check for maximum performance (recommended for Release)
6. Click **"Run workflow"** (green button)

### From GitHub CLI

If you have GitHub CLI installed:

```bash
# Release build with LTO
gh workflow run "Build Windows ARM64 (Manual)" \
  --ref master \
  -f build_type=Release \
  -f enable_lto=true

# Debug build without LTO
gh workflow run "Build Windows ARM64 (Manual)" \
  --ref master \
  -f build_type=Debug \
  -f enable_lto=false
```

## Build Configuration

### Build Types

**Release** (Recommended for distribution)
- Maximum optimization (`/O2`)
- Link-Time Code Generation (if enabled)
- No debug symbols
- Smaller binary size
- Fastest performance on ARM64 devices

**Debug** (For development/debugging)
- No optimization
- Debug symbols included (`/Zi`)
- Larger binary size
- Easier to debug with Visual Studio or WinDbg

### Link-Time Code Generation (LTCG)

**Enabled** (Default, recommended for Release)
- Whole-program optimization
- Better performance (~5-10% faster)
- Longer build time
- Flags: `/O2 /GL /LTCG`

**Disabled** (For faster builds or troubleshooting)
- Standard optimization only
- Faster build time
- Flags: `/O2`

## What Gets Built

The workflow produces:
- Native Windows ARM64 executable (`.exe`)
- Includes all assets (fonts, icons)
- Packaged as `.zip` archive
- Compatible with Windows 11/10 ARM64
- **Guaranteed ARM64** - Architecture verified with `dumpbin`

## Download Artifacts

After the workflow completes:

1. Go to the workflow run page
2. Scroll to **Artifacts** section at the bottom
3. Download: `payroll-and-monitoring-system-windows-arm64-Release.zip`

### Extract and Run

```powershell
# Extract the archive
Expand-Archive payroll-and-monitoring-system-windows-arm64-Release.zip

# Navigate to the folder
cd payroll-and-monitoring-system-windows-arm64-Release

# Run the application (on ARM64 Windows device)
.\payroll-and-monitoring-system.exe
```

### Verify Architecture

You can verify it's an ARM64 executable:

```powershell
# Using dumpbin (if you have Visual Studio installed)
dumpbin /headers payroll-and-monitoring-system.exe | findstr machine
# Should show: AA64 machine (ARM64)
```

## Build Time

Typical build times on GitHub's Windows runners:
- **Release with LTCG:** ~10-15 minutes
- **Release without LTCG:** ~6-10 minutes
- **Debug:** ~5-8 minutes

**Note:** Cross-compilation may be slightly slower than native builds.

## Workflow Details

### Runner
- **OS:** Windows Server 2022 (latest GitHub runner)
- **Build Architecture:** x64 (runner architecture)
- **Target Architecture:** ARM64 (output architecture)
- **Cores:** 2-4 CPU cores available
- **RAM:** ~7 GB available

### Cross-Compilation

This workflow uses **cross-compilation**:
- **Build on:** x64 Windows runner
- **Build for:** ARM64 Windows devices

This is done using the MSVC ARM64 cross-compiler (`amd64_arm64`).

### Dependencies Installed
- MSVC ARM64 cross-compiler (via `msvc-dev-cmd` with `arch: amd64_arm64`)
- Ninja build system (via Chocolatey)
- CMake (pre-installed)

### Build Process
1. Install Ninja build system
2. Setup MSVC ARM64 cross-compiler environment
3. Configure CMake with ARM64 target
4. Build using all available CPU cores
5. Verify executable exists and is ARM64
6. Package with assets
7. Upload artifact

### CMake Configuration
- Generator: Ninja (faster than MSBuild)
- Compiler: MSVC ARM64 cross-compiler
- System: Windows
- Processor: ARM64
- Standard: C++23
- Optimizations: As selected (Release/Debug, LTCG on/off)

## Compiler Information

**MSVC ARM64 Cross-Compiler**
- Part of Visual Studio Build Tools
- Compiles x64 code to ARM64
- Full C++23 support
- Excellent Windows API support

**Cross-Compiler Setup:**
```yaml
uses: ilammy/msvc-dev-cmd@v1
with:
  arch: amd64_arm64  # x64 host, ARM64 target
```

**Optimization Flags:**

With LTCG (Release):
```
/O2      - Maximum optimization
/GL      - Whole program optimization
/LTCG    - Link-time code generation
/DNDEBUG - Disable debug assertions
```

Without LTCG (Release):
```
/O2      - Maximum optimization
/DNDEBUG - Disable debug assertions
```

Debug:
```
/Od      - Disable optimizations
/Zi      - Generate debug information
```

## Performance on ARM64

**Native ARM64 execution:**
- ✅ Full native performance
- ✅ Better battery life than x64 emulation
- ✅ Optimized for ARM architecture

**x64 emulation (not recommended):**
- ❌ Slower performance
- ❌ Higher battery usage
- ❌ Use the x64 build instead if on x64 Windows

## Advantages of Manual Trigger

✅ **You control when builds happen** - No surprise builds on every commit  
✅ **Save GitHub Actions minutes** - Only build when you need to  
✅ **Test specific commits** - Build any branch or commit  
✅ **Multiple configurations** - Try different build types easily  
✅ **No CI noise** - Clean commit history without build checks  

## Comparison with Windows x64 Build

| Feature | ARM64 Workflow | x64 Workflow |
|---------|---------------|--------------|
| **Target Architecture** | ARM64 | x64 |
| **Runner Architecture** | x64 (cross-compile) | x64 (native) |
| **Target Devices** | Surface Pro X, Snapdragon PCs | Traditional Windows PCs |
| **Compilation** | Cross-compilation | Native |
| **Build Time** | Slightly longer | Standard |
| **Use Case** | ARM64 Windows devices | Most Windows PCs |

## System Requirements (For Running the App)

- **OS:** Windows 11 ARM64 or Windows 10 ARM64
- **Architecture:** ARM64 processor (Snapdragon, etc.)
- **RAM:** Depends on your app's needs
- **Visual C++ Redistributable:** May be required (usually pre-installed)

## Troubleshooting

### Build Failed

1. Click on the failed workflow run
2. Click on the failed step (red X)
3. Read the error message
4. Common issues:
   - **CMake configuration failed:** Check CMakeLists.txt syntax
   - **Build failed:** Check for C++ compilation errors in source code
   - **Cross-compilation errors:** Check for x64-specific code
   - **Linker error:** Try disabling LTCG

### App Won't Run on ARM64 Device

If the app won't run on your ARM64 Windows device:

1. **Verify it's ARM64:**
   ```powershell
   dumpbin /headers payroll-and-monitoring-system.exe | findstr machine
   # Should show: AA64 machine (ARM64)
   ```

2. **Check Windows version:**
   - Minimum: Windows 10 ARM64 or Windows 11 ARM64
   - Verify: `systeminfo | findstr /B /C:"OS"`

3. **Install Visual C++ Redistributable:**
   - Download ARM64 version from Microsoft
   - Install and restart

4. **Check Event Viewer:**
   - Open Event Viewer
   - Look under Windows Logs → Application
   - Find crash details

### Running on x64 Windows (Not Recommended)

ARM64 apps can run on x64 Windows via emulation, but:
- ❌ Much slower performance
- ❌ Compatibility issues
- ✅ **Better solution:** Use the Windows x64 workflow instead

## Advanced Options

### Change Artifact Retention

Edit this line in the workflow:
```yaml
retention-days: 90  # Change to desired number of days (1-90)
```

### Custom CMake Flags

To add custom CMake flags, edit the workflow file:

```yaml
cmake -B build `
  -DCMAKE_BUILD_TYPE=${{ inputs.build_type }} `
  -G Ninja `
  -DCMAKE_SYSTEM_NAME=Windows `
  -DCMAKE_SYSTEM_PROCESSOR=ARM64 `
  -DYOUR_CUSTOM_FLAG=ON
```

### Different ARM64 Compiler

The default is `amd64_arm64` (x64 host → ARM64 target). For other options:

```yaml
# ARM64 host → ARM64 target (if running on ARM64 runner)
arch: arm64

# x86 host → ARM64 target
arch: x86_arm64
```

## Performance Comparison

| Configuration | Build Time | Binary Size | Performance on ARM64 |
|--------------|------------|-------------|----------------------|
| Release + LTCG | ~12 min | Smallest | Fastest |
| Release (no LTCG) | ~8 min | Small | Fast |
| Debug | ~6 min | Largest | Slowest |

## Notes

- **No automatic triggers:** This workflow never runs automatically
- **ARM64 only:** Not compatible with traditional x64 Windows PCs
- **Windows 11 recommended:** Best ARM64 support and performance
- **Cross-compiled:** Built on x64 runner, targets ARM64

## File Locations

- **Workflow:** `.github/workflows/build-windows-arm64.yml`
- **Documentation:** `.github/workflows/README-windows-arm64.md`

## Best Practices

1. **Use Release builds for distribution**
   - Smaller size
   - Better performance
   - No debug overhead

2. **Enable LTCG for Release**
   - Significant performance improvement
   - Worth the extra build time

3. **Test on actual ARM64 device**
   - Don't rely on x64 emulation
   - Test on Surface Pro X or similar

4. **Consider code signing**
   - Prevents Windows Defender warnings
   - Builds user trust
   - Requires code signing certificate

## Target Audience

Build ARM64 version when:
- ✅ Users have Surface Pro X or similar ARM64 devices
- ✅ Want native performance on ARM64 Windows
- ✅ Developing for Windows on ARM ecosystem
- ✅ Need better battery life than x64 emulation

Use x64 version when:
- ✅ Users have traditional Windows PCs
- ✅ Maximum compatibility needed
- ✅ Not targeting ARM64 specifically

## Support

If you encounter issues:
1. Check the workflow run logs
2. Look at the "Build summary" at the end
3. Verify architecture with `dumpbin`
4. Check Windows Event Viewer for runtime errors
5. Test on actual ARM64 Windows device

---

**Quick Start:** Actions → Build Windows ARM64 (Manual) → Run workflow → Download artifact

**Remember:** This builds for ARM64 Windows devices (Surface Pro X, etc.). For regular Windows PCs, use the x64 workflow!

