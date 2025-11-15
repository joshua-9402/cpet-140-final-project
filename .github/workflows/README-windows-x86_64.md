# Windows x64 Build Workflow

## Overview

This workflow builds the **structuracost** application specifically for **Windows x64** (64-bit Windows).

## Key Features

- ✅ **Manual trigger only** - Runs only when you explicitly trigger it
- ✅ **Windows 10/11 compatible** - Works on modern Windows versions
- ✅ **x64 architecture** - 64-bit Windows executables
- ✅ **Configurable** - Choose Release or Debug build
- ✅ **Optional LTO** - Enable/disable Link-Time Code Generation (LTCG)
- ✅ **90-day retention** - Artifacts kept for 3 months
- ✅ **Architecture verification** - Confirms x64 build with `dumpbin`
- ✅ **MSVC compiler** - Microsoft Visual C++ compiler

## Compatibility

This build works on:
- ✅ **Windows 10** - All versions (64-bit)
- ✅ **Windows 11** - All versions (64-bit)
- ✅ **Windows Server 2016+** - Server editions
- ❌ **Windows 32-bit** - Not supported (use x86 build if needed)
- ❌ **Windows 7/8/8.1** - May work but not officially supported

## How to Trigger a Build

### From GitHub Website

1. Go to your repository on GitHub
2. Click the **Actions** tab
3. Select **"Windows"** from the workflow list (left side)
4. Click the **"Run workflow"** button (right side)
5. Configure options:
   - **Branch:** Choose the branch to build (usually `master` or `main`)
   - **Architecture:** Choose `x64`
   - **Build type:** Choose `Release` (optimized) or `Debug` (with debug symbols)
   - **Enable Link-Time Optimization:** Check for maximum performance (recommended for Release)
6. Click **"Run workflow"** (green button)

### From GitHub CLI

If you have GitHub CLI installed:

```bash
# Release build with LTO
gh workflow run "Windows" \
  --ref master \
  -f architecture=x64 \
  -f build_type=Release \
  -f enable_lto=true

# Debug build without LTO
gh workflow run "Windows" \
  --ref master \
  -f architecture=x64 \
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
- Fastest performance

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
- Native Windows x64 executable (`.exe`)
- Includes all assets (fonts, icons)
- Packaged as `.zip` archive
- Compatible with Windows 10/11 (64-bit)
- **Guaranteed x64** - Architecture verified with `dumpbin`

## Download Artifacts

After the workflow completes:

1. Go to the workflow run page
2. Scroll to **Artifacts** section at the bottom
3. Download: `payroll-and-monitoring-system-windows-x64-Release.zip`

### Extract and Run

```powershell
# Extract the archive
Expand-Archive payroll-and-monitoring-system-windows-x64-Release.zip

# Navigate to the folder
cd payroll-and-monitoring-system-windows-x64-Release

# Run the application
.\payroll-and-monitoring-system.exe
```

### Verify Architecture

You can verify it's a 64-bit executable:

```powershell
# Using dumpbin (if you have Visual Studio installed)
dumpbin /headers payroll-and-monitoring-system.exe | findstr machine
# Should show: 8664 machine (x64)

# Or check file properties
# Right-click exe → Properties → Details tab
# Look for "File description" or use:
[System.Diagnostics.FileVersionInfo]::GetVersionInfo("payroll-and-monitoring-system.exe")
```

## Build Time

Typical build times on GitHub's Windows runners:
- **Release with LTCG:** ~8-12 minutes
- **Release without LTCG:** ~5-8 minutes
- **Debug:** ~4-6 minutes

## Workflow Details

### Runner
- **OS:** Windows Server 2022 (latest GitHub runner)
- **Architecture:** x64
- **Cores:** 2-4 CPU cores available
- **RAM:** ~7 GB available

### Dependencies Installed
- MSVC (Microsoft Visual C++ - pre-installed)
- Ninja build system (via Chocolatey)
- CMake (pre-installed)

### Build Process
1. Install Ninja build system
2. Setup MSVC compiler
3. Configure CMake with Ninja generator
4. Build using all available CPU cores
5. Verify executable exists and is x64
6. Package with assets
7. Upload artifact

### CMake Configuration
- Generator: Ninja (faster than MSBuild)
- Compiler: MSVC (cl.exe)
- Standard: C++23
- Optimizations: As selected (Release/Debug, LTCG on/off)

## Compiler Information

**MSVC (Microsoft Visual C++)**
- Part of Visual Studio Build Tools
- Industry-standard Windows C++ compiler
- Excellent Windows API support
- Best optimization for Windows

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

## Advantages of Manual Trigger

✅ **You control when builds happen** - No surprise builds on every commit  
✅ **Save GitHub Actions minutes** - Only build when you need to  
✅ **Test specific commits** - Build any branch or commit  
✅ **Multiple configurations** - Try different build types easily  
✅ **No CI noise** - Clean commit history without build checks  

## Comparison with Other Windows Builds

This is the **manual Windows x64 workflow** optimized for on-demand builds.

| Feature | Manual Workflow | Automated Workflow |
|---------|----------------|-------------------|
| **Trigger** | Manual only | Automatic on push |
| **Retention** | 90 days | 30 days |
| **Configurability** | High (choose build type, LTO) | Fixed (Release with LTO) |
| **Use Case** | On-demand, testing, distribution | CI/CD, quick validation |

## System Requirements (For Running the App)

- **OS:** Windows 10 (64-bit) or Windows 11
- **Architecture:** x64 (64-bit processor)
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
   - **Executable not found:** Build may have failed silently
   - **Linker error:** Try disabling LTCG

### Missing DLLs When Running

If the app complains about missing DLLs:

1. **Install Visual C++ Redistributable:**
   - Download from Microsoft's website
   - Install the x64 version matching your Visual Studio version

2. **Check for missing dependencies:**
   - Use [Dependency Walker](http://www.dependencywalker.com/) or
   - Use [Dependencies](https://github.com/lucasg/Dependencies) (modern alternative)

### Antivirus/Windows Defender Blocks App

If Windows blocks the app:
1. The app is unsigned (expected for development builds)
2. Add exception in Windows Defender
3. Or: Sign the executable with a code signing certificate

### App Won't Run on Windows 10

Verify:
1. You're on 64-bit Windows 10 (not 32-bit)
2. Visual C++ Redistributable is installed
3. Check Event Viewer for crash details

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
  -DCMAKE_C_COMPILER=cl `
  -DCMAKE_CXX_COMPILER=cl `
  -DYOUR_CUSTOM_FLAG=ON
```

### Build for Different Windows Versions

The default build works on Windows 10+. For older Windows:

```yaml
# Add to CMake configuration
-DCMAKE_SYSTEM_VERSION=10.0.17763.0  # Windows 10 1809
```

### Enable Static Linking

To create a fully standalone executable (no DLL dependencies):

```yaml
# Add to CMake flags
-DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded"
```

## Performance Comparison

| Configuration | Build Time | Binary Size | Performance |
|--------------|------------|-------------|-------------|
| Release + LTCG | ~10 min | Smallest | Fastest |
| Release (no LTCG) | ~6 min | Small | Fast |
| Debug | ~5 min | Largest | Slowest |

## Notes

- **No automatic triggers:** This workflow never runs automatically
- **x64 only:** Not compatible with 32-bit Windows (use x86 build if needed)
- **Windows 10+ recommended:** May work on older versions but not tested
- **Native build:** Built on Windows Server runner with MSVC

## File Locations

- **Workflow:** `.github/workflows/build-windows-x64.yml`
- **Documentation:** `.github/workflows/README-windows-x64.md`

## Best Practices

1. **Use Release builds for distribution**
   - Smaller size
   - Better performance
   - No debug overhead

2. **Enable LTCG for Release**
   - Significant performance improvement
   - Worth the extra build time

3. **Test on actual Windows 10/11**
   - Don't assume it works everywhere
   - Test both Windows 10 and 11 if possible

4. **Consider code signing**
   - Prevents Windows Defender warnings
   - Builds user trust
   - Requires code signing certificate

## Packaging for Distribution

### Create Installer

Use tools like:
- **Inno Setup** - Free, popular installer creator
- **WiX Toolset** - Windows Installer XML
- **NSIS** - Nullsoft Scriptable Install System

### Portable Version

The .zip artifact is already portable:
- No installation needed
- Can run from USB drive
- Just extract and run

## Support

If you encounter issues:
1. Check the workflow run logs
2. Look at the "Build summary" at the end
3. Verify architecture with `dumpbin`
4. Check Windows Event Viewer for runtime errors

---

**Quick Start:** Actions → Build Windows x64 (Manual) → Run workflow → Download artifact

**Remember:** Test the executable on actual Windows 10/11 systems before distributing!

