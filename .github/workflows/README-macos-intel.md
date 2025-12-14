# macOS Intel Build Workflow

## Overview

This workflow builds the **structuracost** application specifically for **macOS Sequoia (15.0+) on Intel x86_64** processors.

## Supported Configurations

### macOS Versions
- **macOS Sequoia (15.x)** - Intel Macs (minimum)

### Architectures
- **x86_64** (Intel 64-bit)

### Target Devices
**Intel Macs:**
- MacBook Air (2015-2020)
- MacBook Pro (2015-2020)
- Mac mini (2014-2020)
- iMac (2015-2020)
- iMac Pro (2017-2021)
- Mac Pro (2013-2019)

## Key Features

- ✅ **Manual trigger only** - Runs only when you explicitly trigger it
- ✅ **macOS Sequoia compatible** - Targets macOS 15.0+ (Sequoia and newer)
- ✅ **Intel x86_64 native** - Built specifically for Intel processors
- ✅ **Configurable** - Choose Release or Debug build
- ✅ **Optional LTO** - Enable/disable Link-Time Optimization
- ✅ **90-day retention** - Artifacts kept for 3 months
- ✅ **Architecture verification** - Confirms x86_64 build with `lipo`

## Compatibility

This build works on:
- ✅ **macOS Sequoia (15.x)** - Intel Macs
- ❌ **Apple Silicon Macs** - Use the ARM64 workflow instead

## How to Trigger a Build

### From GitHub Website

1. Go to your repository on GitHub
2. Click the **Actions** tab
3. Select **"macOS"** from the workflow list (left side)
4. Click the **"Run workflow"** button (right side)
5. Configure options:
   - **Branch:** Choose the branch to build (usually `master` or `main`)
   - **Target:** Choose `intel` (Intel x86_64)
   - **Build type:** Choose `Release` (optimized) or `Debug` (with debug symbols)
   - **Enable Link-Time Optimization:** Check for maximum performance (recommended for Release)
6. Click **"Run workflow"** (green button)

### From GitHub CLI

If you have GitHub CLI installed:

```bash
# Release build with LTO for Intel
gh workflow run "macOS" \
  --ref master \
  -f target=intel \
  -f build_type=Release \
  -f enable_lto=true

# Debug build without LTO
gh workflow run "macOS" \
  --ref master \
  -f target=intel \
  -f build_type=Debug \
  -f enable_lto=false
```

## Build Configuration

### Build Types

**Release** (Recommended for distribution)
- Maximum optimization (`-O3`)
- Link-Time Optimization (if enabled)
- No debug symbols
- Smaller binary size
- Fastest performance

**Debug** (For development/debugging)
- No optimization
- Debug symbols included
- Larger binary size
- Easier to debug with tools like `lldb`

### Link-Time Optimization (LTO)

**Enabled** (Default, recommended for Release)
- Whole-program optimization
- Better performance (~5-10% faster)
- Longer build time
- Flags: `-O3 -flto -DNDEBUG`

**Disabled** (For faster builds or troubleshooting)
- Standard optimization only
- Faster build time
- Flags: `-O3 -DNDEBUG`

## What Gets Built

The workflow produces:
- Native macOS Intel x86_64 executable or `.app` bundle
- Includes all assets (fonts, icons)
- Packaged as `.tar.gz` archive
- Compatible with macOS Big Sur (11.0+)
- **Guaranteed x86_64** - Architecture verified with `lipo`

## Download Artifacts

After the workflow completes:

1. Go to the workflow run page
2. Scroll to **Artifacts** section at the bottom
3. Download: `structuracost-macos-intel-v1.0.0.tar.gz` (example)

**Artifact Format:** `structuracost-macos-intel-{version}.tar.gz`

**Retention:** 90 days

### Extract and Run

```bash
# Extract the archive
tar -xzf structuracost-macos-intel-v1.0.0.tar.gz

# If it's an app bundle
open structuracost.app

# If it's a standalone executable
./structuracost
```

### Verify Architecture

You can verify it's an Intel binary:

```bash
# Check the architecture
file payroll-and-monitoring-system.app/Contents/MacOS/payroll-and-monitoring-system
# Output should show: Mach-O 64-bit executable x86_64

# Or use lipo
lipo -info payroll-and-monitoring-system.app/Contents/MacOS/payroll-and-monitoring-system
# Output: Non-fat file: ... is architecture: x86_64
```

## Build Time

Typical build times on GitHub's Intel macOS runners:
- **Release with LTO:** ~8-12 minutes
- **Release without LTO:** ~5-8 minutes
- **Debug:** ~4-6 minutes

## Comparison with ARM64 Workflow

| Feature | Intel Workflow | ARM64 Workflow |
|---------|---------------|----------------|
| **Runner** | macos-13 | macos-14 |
| **Architecture** | x86_64 | ARM64 |
| **macOS Version** | 11.0+ (Big Sur) | 14.0+ (Sonoma) |
| **Compatible Macs** | Intel Macs | Apple Silicon Macs |
| **Deployment Target** | macOS 11.0 | macOS 14.0 |
| **Architecture Flag** | `-DCMAKE_OSX_ARCHITECTURES=x86_64` | (native) |

## Workflow Details

### Runner
- **OS:** macOS 13.x (Ventura) - Runner environment
- **Builds for:** macOS 11.0+ (Big Sur and newer) - Via deployment target
- **Architecture:** x86_64 (Intel)
- **Cores:** 3-4 CPU cores available

**Important:** The runner uses macOS 13, but sets `CMAKE_OSX_DEPLOYMENT_TARGET=11.0` to ensure the compiled binary is compatible with macOS Big Sur 11.0 and all newer versions.

### Dependencies Installed
- CMake (latest from Homebrew)
- Ninja build system
- FreeType library
- Clang/LLVM (pre-installed)

### Build Process
1. Install dependencies
2. Configure CMake with Ninja generator
3. Explicitly set x86_64 architecture
4. Build using all available CPU cores
5. Verify executable exists and architecture
6. Package with assets
7. Upload artifact

### CMake Configuration
- Generator: Ninja (faster than Make)
- Deployment target: macOS 11.0 (Big Sur)
- Architecture: x86_64 (explicitly set)
- Standard: C++23
- Optimizations: As selected (Release/Debug, LTO on/off)

## Architecture Verification

This workflow includes automatic architecture verification:

```bash
# Checks that the binary is x86_64
file build/payroll-and-monitoring-system
lipo -info build/payroll-and-monitoring-system
```

If the architecture is wrong, the build will fail with clear error messages.

## Troubleshooting

### Build Failed

1. Click on the failed workflow run
2. Click on the failed step (it will have a red X)
3. Read the error message
4. Common issues:
   - **CMake configuration failed:** Check CMakeLists.txt syntax
   - **Build failed:** Check for C++ compilation errors in source code
   - **Executable not found:** Build may have failed silently
   - **Wrong architecture:** Check CMake configuration

### Artifact Download Issues

- Artifacts expire after 90 days
- Make sure the workflow completed successfully (green checkmark)
- Artifacts appear at the bottom of the workflow run page

### App Won't Run on Intel Mac

If the app won't run:
1. Check if you're on an Intel Mac: `uname -m` should show `x86_64`
2. Verify macOS version: `sw_vers` should show 11.0 or higher
3. Check Gatekeeper: Right-click app → Open (first time only)
4. Verify architecture: `lipo -info payroll-and-monitoring-system.app/Contents/MacOS/payroll-and-monitoring-system`

## Advanced Options

### Change Deployment Target

To support older macOS versions, edit the workflow:

```yaml
-DCMAKE_OSX_DEPLOYMENT_TARGET=10.15  # For macOS Catalina
-DCMAKE_OSX_DEPLOYMENT_TARGET=10.13  # For macOS High Sierra
```

**Note:** Lower targets may require testing to ensure compatibility.

### Custom CMake Flags

To add custom CMake flags, edit the workflow file:

```yaml
cmake -B build \
  -DCMAKE_BUILD_TYPE=${{ inputs.build_type }} \
  -G Ninja \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
  -DCMAKE_OSX_ARCHITECTURES=x86_64 \
  -DYOUR_CUSTOM_FLAG=ON
```

### Change Artifact Retention

Edit this line in the workflow:
```yaml
retention-days: 90  # Change to desired number of days (1-90)
```

## Performance Comparison

| Configuration | Build Time | Binary Size | Performance |
|--------------|------------|-------------|-------------|
| Release + LTO | ~10 min | Smallest | Fastest |
| Release (no LTO) | ~6 min | Small | Fast |
| Debug | ~5 min | Largest | Slowest |

## Notes

- **No automatic triggers:** This workflow never runs automatically
- **Intel only:** Not compatible with Apple Silicon Macs (use ARM64 workflow)
- **Big Sur minimum:** Built for macOS 11.0+, compatible with newer versions
- **Native build:** Built on Intel runner, architecture explicitly set to x86_64
- **Architecture verified:** Uses `lipo` to confirm x86_64 binary

## System Requirements (For Running the App)

- **macOS:** Big Sur 11.0 or newer
- **CPU:** Intel x86_64 processor
- **RAM:** Depends on your app's needs
- **Not compatible with:** Apple Silicon Macs (use ARM64 build instead)

## Both Workflows Available

You now have two macOS workflows:

| Workflow | For | Architecture | macOS Version |
|----------|-----|--------------|---------------|
| **Build macOS ARM64 (Manual)** | Apple Silicon Macs | ARM64 | Sonoma 14.0+ |
| **Build macOS Intel (Manual)** | Intel Macs | x86_64 | Big Sur 11.0+ |

Choose the appropriate one based on your target Mac hardware.

## Support

If you encounter issues:
1. Check the workflow run logs
2. Look at the "Build summary" at the end
3. Verify architecture with `lipo -info`
4. Check CMakeLists.txt is valid

---

**Quick Start:** Actions → Build macOS Intel (Manual) → Run workflow → Download artifact

