# macOS ARM64 Build Workflow

## Overview

This workflow builds the payroll-and-monitoring-system specifically for **macOS Sonoma ARM64 (Apple Silicon)**.

## Key Features

- ✅ **Manual trigger only** - Runs only when you explicitly trigger it
- ✅ **macOS Sonoma optimized** - Targets macOS 14.0+ (Sonoma)
- ✅ **Apple Silicon native** - Built specifically for ARM64 architecture
- ✅ **Configurable** - Choose Release or Debug build
- ✅ **Optional LTO** - Enable/disable Link-Time Optimization
- ✅ **90-day retention** - Artifacts kept for 3 months

## How to Trigger a Build

### From GitHub Website

1. Go to your repository on GitHub
2. Click the **Actions** tab
3. Select **"Build macOS ARM64 (Manual)"** from the workflow list (left side)
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
gh workflow run "Build macOS ARM64 (Manual)" \
  --ref master \
  -f build_type=Release \
  -f enable_lto=true

# Debug build without LTO
gh workflow run "Build macOS ARM64 (Manual)" \
  --ref master \
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
- Native macOS ARM64 executable or `.app` bundle
- Includes all assets (fonts, icons)
- Packaged as `.tar.gz` archive
- Compatible with macOS Sonoma (14.0+)

## Download Artifacts

After the workflow completes:

1. Go to the workflow run page
2. Scroll to **Artifacts** section at the bottom
3. Download: `payroll-and-monitoring-system-macos-arm64-Release.tar.gz` (or Debug)

### Extract and Run

```bash
# Extract the archive
tar -xzf payroll-and-monitoring-system-macos-arm64-Release.tar.gz

# If it's an app bundle
open payroll-and-monitoring-system.app

# If it's a standalone executable
./payroll-and-monitoring-system
```

## Build Time

Typical build times on GitHub's macOS runners:
- **Release with LTO:** ~8-12 minutes
- **Release without LTO:** ~5-8 minutes
- **Debug:** ~4-6 minutes

## Troubleshooting

### Build Failed

1. Click on the failed workflow run
2. Click on the failed step (it will have a red X)
3. Read the error message
4. Common issues:
   - **CMake configuration failed:** Check CMakeLists.txt syntax
   - **Build failed:** Check for C++ compilation errors in source code
   - **Executable not found:** Build may have failed silently

### Artifact Download Issues

- Artifacts expire after 90 days
- Make sure the workflow completed successfully (green checkmark)
- Artifacts appear at the bottom of the workflow run page

### Can't Find Workflow

- Make sure the `.github/workflows/build-macos-arm64.yml` file is committed
- It must be in the repository's default branch to appear in Actions

## Workflow Details

### Runner
- **OS:** macOS (latest, currently Sonoma or newer)
- **Architecture:** ARM64 (Apple Silicon - M1/M2/M3)
- **Cores:** 3-4 CPU cores available

### Dependencies Installed
- CMake (latest from Homebrew)
- Ninja build system
- FreeType library
- Clang/LLVM (pre-installed)

### Build Process
1. Install dependencies
2. Configure CMake with Ninja generator
3. Build using all available CPU cores
4. Verify executable exists
5. Package with assets
6. Upload artifact

### CMake Configuration
- Generator: Ninja (faster than Make)
- Deployment target: macOS 14.0 (Sonoma)
- Standard: C++20
- Optimizations: As selected (Release/Debug, LTO on/off)

## Advanced Options

### Custom CMake Flags

To add custom CMake flags, edit the workflow file:

```yaml
cmake -B build \
  -DCMAKE_BUILD_TYPE=${{ inputs.build_type }} \
  -G Ninja \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0 \
  -DYOUR_CUSTOM_FLAG=ON
```

### Change Artifact Retention

Edit this line in the workflow:
```yaml
retention-days: 90  # Change to desired number of days (1-90)
```

### Build for Older macOS Versions

Change the deployment target:
```yaml
-DCMAKE_OSX_DEPLOYMENT_TARGET=13.0  # For macOS Ventura
-DCMAKE_OSX_DEPLOYMENT_TARGET=12.0  # For macOS Monterey
```

## Performance Comparison

| Configuration | Build Time | Binary Size | Performance |
|--------------|------------|-------------|-------------|
| Release + LTO | ~10 min | Smallest | Fastest |
| Release (no LTO) | ~6 min | Small | Fast |
| Debug | ~5 min | Largest | Slowest |

## Notes

- **No automatic triggers:** This workflow never runs automatically
- **Apple Silicon only:** Not compatible with Intel Macs (use separate workflow for x64)
- **Sonoma minimum:** Built for macOS 14.0+, may work on newer versions
- **Native build:** Built on ARM64 runner, no cross-compilation

## Support

If you encounter issues:
1. Check the workflow run logs
2. Look at the "Build summary" at the end
3. Verify all source files are committed
4. Check CMakeLists.txt is valid

---

**Quick Start:** Actions → Build macOS ARM64 (Manual) → Run workflow → Download artifact

