# macOS Legacy Build Workflow

## Overview

This workflow builds the payroll-and-monitoring-system for **older macOS versions (Catalina 10.15, Mojave 10.14, High Sierra 10.13)** on Intel x86_64 processors.

## Key Features

- ✅ **Manual trigger only** - Runs only when you explicitly trigger it
- ✅ **Selectable deployment target** - Choose minimum macOS version (10.15, 10.14, or 10.13)
- ✅ **Legacy macOS compatible** - Targets older Intel Macs
- ✅ **Intel x86_64 native** - Built specifically for Intel processors
- ✅ **Configurable** - Choose Release or Debug build
- ✅ **Optional LTO** - Enable/disable Link-Time Optimization (disabled by default for compatibility)
- ✅ **C++20 standard** - Uses C++20 instead of C++23 for better compatibility
- ✅ **90-day retention** - Artifacts kept for 3 months
- ✅ **Deployment verification** - Confirms minimum macOS version with `otool`

## macOS Version Compatibility

| Deployment Target | macOS Version | Release Year | Compatible With |
|-------------------|---------------|--------------|-----------------|
| **10.15** | Catalina | 2019 | Catalina, Big Sur, Monterey, Ventura, Sonoma |
| **10.14** | Mojave | 2018 | Mojave and all newer versions |
| **10.13** | High Sierra | 2017 | High Sierra and all newer versions |

## Important Notes

### Why C++20 instead of C++23?

Older macOS versions (Catalina and earlier) may have:
- Older system libraries
- Older C++ runtime
- Limited C++23 feature support

Using **C++20** ensures better compatibility with these older systems while still providing modern C++ features.

### LTO Disabled by Default

Link-Time Optimization (LTO) is **disabled by default** for legacy builds because:
- Older linkers may not fully support LTO
- Some older macOS versions have buggy LTO implementations
- Better compatibility is more important than maximum performance for legacy targets

You can enable it manually if needed, but test thoroughly on the target OS.

## How to Trigger a Build

### From GitHub Website

1. Go to your repository on GitHub
2. Click the **Actions** tab
3. Select **"Build macOS Legacy (Manual)"** from the workflow list (left side)
4. Click the **"Run workflow"** button (right side)
5. Configure options:
   - **Branch:** Choose the branch to build (usually `master` or `main`)
   - **Build type:** Choose `Release` or `Debug`
   - **Enable Link-Time Optimization:** Usually leave **unchecked** for legacy compatibility
   - **Minimum macOS version:** Choose your target:
     - `10.15` - macOS Catalina (2019)
     - `10.14` - macOS Mojave (2018)
     - `10.13` - macOS High Sierra (2017)
6. Click **"Run workflow"** (green button)

### From GitHub CLI

```bash
# Build for Catalina (10.15) - Release without LTO
gh workflow run "Build macOS Legacy (Manual)" \
  --ref master \
  -f build_type=Release \
  -f enable_lto=false \
  -f deployment_target=10.15

# Build for Mojave (10.14) - Release without LTO
gh workflow run "Build macOS Legacy (Manual)" \
  --ref master \
  -f build_type=Release \
  -f enable_lto=false \
  -f deployment_target=10.14

# Build for High Sierra (10.13) - Debug
gh workflow run "Build macOS Legacy (Manual)" \
  --ref master \
  -f build_type=Debug \
  -f enable_lto=false \
  -f deployment_target=10.13
```

## Choosing the Right Deployment Target

### Rule of Thumb

Choose the **oldest** macOS version you want to support:

- **10.15 (Catalina)** - Most common choice for legacy support
  - Last version to support many 32-bit apps
  - Good balance of compatibility and features
  - Recommended for most legacy builds

- **10.14 (Mojave)** - For older hardware
  - Supports Macs from 2012 and later
  - Some features may be limited

- **10.13 (High Sierra)** - Maximum legacy compatibility
  - Supports very old Macs (2009-2012 era)
  - More limited features and libraries
  - Test thoroughly on actual hardware

### Important

The binary will work on:
- The chosen deployment target version
- **ALL newer macOS versions**

Example: If you choose `10.15`, the app will work on Catalina, Big Sur, Monterey, Ventura, and Sonoma.

## Build Configuration

### Build Types

**Release** (Recommended for distribution)
- Optimization: `-O3`
- Link-Time Optimization: Optional (disabled by default)
- No debug symbols
- Smaller binary size
- Good performance

**Debug** (For development/debugging)
- No optimization
- Debug symbols included
- Larger binary size
- Easier to debug with `lldb`

### Link-Time Optimization (LTO)

**Disabled** (Default, recommended for legacy)
- Standard optimization only
- Better compatibility with older linkers
- Flags: `-O3 -DNDEBUG`

**Enabled** (Use with caution)
- Whole-program optimization
- May have compatibility issues on older macOS
- Test on actual target hardware
- Flags: `-O3 -flto -DNDEBUG`

## What Gets Built

The workflow produces:
- Native macOS Intel x86_64 executable or `.app` bundle
- **C++20 standard** (not C++23) for legacy compatibility
- Includes all assets (fonts, icons)
- Packaged as `.tar.gz` archive
- Compatible with selected macOS version and all newer versions
- **Guaranteed x86_64** - Architecture verified with `lipo`
- **Deployment target verified** - Checked with `otool`

## Download Artifacts

After the workflow completes:

1. Go to the workflow run page
2. Scroll to **Artifacts** section at the bottom
3. Download: `payroll-and-monitoring-system-macos-legacy-10.15-Release.tar.gz` (or your chosen version)

### Extract and Run

```bash
# Extract the archive
tar -xzf payroll-and-monitoring-system-macos-legacy-10.15-Release.tar.gz

# If it's an app bundle
open payroll-and-monitoring-system.app

# If it's a standalone executable
./payroll-and-monitoring-system
```

### Verify Compatibility

```bash
# Check the architecture
file payroll-and-monitoring-system.app/Contents/MacOS/payroll-and-monitoring-system
# Output should show: Mach-O 64-bit executable x86_64

# Verify with lipo
lipo -info payroll-and-monitoring-system.app/Contents/MacOS/payroll-and-monitoring-system
# Output: Non-fat file: ... is architecture: x86_64

# Check minimum macOS version
otool -l payroll-and-monitoring-system.app/Contents/MacOS/payroll-and-monitoring-system | grep -A 3 LC_VERSION_MIN_MACOSX
# Should show your selected deployment target (e.g., 10.15)
```

## Build Time

Typical build times on GitHub's Intel macOS runners:
- **Release without LTO:** ~5-8 minutes
- **Release with LTO:** ~8-12 minutes (if enabled)
- **Debug:** ~4-6 minutes

## Comparison with Other macOS Workflows

| Feature | Legacy Workflow | Intel Workflow | ARM64 Workflow |
|---------|----------------|----------------|----------------|
| **Runner** | macos-13 | macos-13 | macos-14 |
| **Architecture** | x86_64 | x86_64 | ARM64 |
| **Min macOS** | 10.13-10.15 (selectable) | 11.0 (Big Sur) | 14.0 (Sonoma) |
| **C++ Standard** | C++20 | C++23 | C++23 |
| **LTO Default** | Disabled | Enabled | Enabled |
| **Best For** | Old Intel Macs (2012-2019) | Modern Intel Macs | Apple Silicon |

## All macOS Workflows Available

You now have **three** macOS workflows:

| Workflow | Target Hardware | macOS Versions | C++ Std |
|----------|----------------|----------------|---------|
| **Build macOS ARM64** | Apple Silicon (M1/M2/M3) | Sonoma 14.0+ | C++23 |
| **Build macOS Intel** | Modern Intel Macs | Big Sur 11.0+ | C++23 |
| **Build macOS Legacy** | Old Intel Macs | Catalina/Mojave/High Sierra | C++20 |

## Workflow Details

### Runner
- **OS:** macOS 13.x (Ventura) - Build environment
- **Builds for:** macOS 10.13-10.15 (selectable) - Via deployment target
- **Architecture:** x86_64 (Intel)
- **Cores:** 3-4 CPU cores available

**Important:** The runner uses macOS 13, but sets `CMAKE_OSX_DEPLOYMENT_TARGET` to your selected version (10.15/10.14/10.13) to ensure the compiled binary is compatible with older macOS versions.

### Dependencies Installed
- CMake (latest from Homebrew)
- Ninja build system
- FreeType library
- Clang/LLVM (pre-installed)

### Build Process
1. Install dependencies
2. Configure CMake with selected deployment target
3. Set C++20 standard (instead of C++23)
4. Explicitly set x86_64 architecture
5. Build using all available CPU cores
6. Verify executable exists, architecture, and deployment target
7. Package with assets
8. Upload artifact with version-specific name

### CMake Configuration
- Generator: Ninja (faster than Make)
- Deployment target: User-selectable (10.15/10.14/10.13)
- Architecture: x86_64 (explicitly set)
- Standard: **C++20** (for legacy compatibility)
- Optimizations: As selected (Release/Debug, LTO usually off)

## Testing on Legacy macOS

### Recommended Testing

Always test on actual hardware running the target macOS version:

1. **Build for your target** (e.g., 10.15 Catalina)
2. **Download the artifact**
3. **Copy to a Mac running that macOS version**
4. **Test all features thoroughly**

### Virtual Machines

You can also test using VMs:
- VMware Fusion
- Parallels Desktop
- VirtualBox (limited macOS support)

Install the target macOS version in a VM and test there.

## Troubleshooting

### Build Failed

1. Click on the failed workflow run
2. Click on the failed step (red X)
3. Common issues:
   - **C++23 features used:** Code may use C++23 features not in C++20 - refactor to C++20
   - **Newer API calls:** Code may use APIs only in newer macOS - add version checks
   - **LTO issues:** Disable LTO if compilation/linking fails

### App Won't Run on Target macOS

If the app won't run on the target macOS version:

1. **Verify deployment target:**
   ```bash
   otool -l payroll-and-monitoring-system.app/Contents/MacOS/payroll-and-monitoring-system | grep -A 3 LC_VERSION_MIN_MACOSX
   ```

2. **Check for newer API usage:**
   - Your code might use APIs not available in older macOS
   - Add `@available` checks or conditional compilation

3. **Check console logs:**
   - Open Console.app on the target Mac
   - Look for crash reports or error messages

4. **Gatekeeper issues:**
   - Right-click app → Open (first time only)
   - Or: System Preferences → Security & Privacy → Allow

### Known Limitations

**Catalina (10.15) and older:**
- 32-bit app support removed in Catalina
- Some newer frameworks unavailable
- Older OpenGL version
- Limited Metal support on some hardware

**Mojave (10.14) and older:**
- Dark Mode limited or unavailable
- Some security features missing
- Older SDK features

**High Sierra (10.13) and older:**
- Very limited Metal support
- Older graphics APIs
- Some modern C++ library features missing

## Advanced Options

### Change C++ Standard

If you need C++17 for even older compatibility, edit the workflow:

```yaml
-DCMAKE_CXX_STANDARD=17  # C++17 for maximum legacy compatibility
```

### Enable LTO (Test First!)

If you want to enable LTO for legacy builds:

1. Edit the workflow or select "Enable LTO" when triggering
2. **Test thoroughly** on actual target hardware
3. Watch for linker errors or crashes

### Add Even Older macOS Support

For macOS 10.12 (Sierra) or older, edit the workflow:

```yaml
deployment_target:
  type: choice
  options:
    - '10.15'
    - '10.14'
    - '10.13'
    - '10.12'  # Sierra
    - '10.11'  # El Capitan
```

**Warning:** Very old versions may have significant compatibility issues.

## Performance Comparison

| Configuration | Build Time | Compatibility | Performance |
|--------------|------------|---------------|-------------|
| Legacy (C++20, no LTO) | ~6 min | Best | Good |
| Legacy (C++20, LTO) | ~10 min | Good | Better |
| Modern (C++23, LTO) | ~10 min | Newer macOS only | Best |

## System Requirements (For Running the App)

Depends on deployment target chosen:

### 10.15 (Catalina)
- **macOS:** Catalina 10.15 or newer
- **CPU:** Intel x86_64 (2012 or newer Macs)
- **RAM:** Depends on app's needs
- **Not compatible with:** macOS 10.14 or older, Apple Silicon (use ARM64 build)

### 10.14 (Mojave)
- **macOS:** Mojave 10.14 or newer
- **CPU:** Intel x86_64 (2012 or newer Macs)
- **Not compatible with:** macOS 10.13 or older

### 10.13 (High Sierra)
- **macOS:** High Sierra 10.13 or newer
- **CPU:** Intel x86_64 (2009-2012 era Macs supported)
- **Not compatible with:** macOS 10.12 or older

## Best Practices

1. **Choose the oldest macOS you actually need to support**
   - Don't go older than necessary
   - Each older version adds testing burden

2. **Test on actual hardware**
   - VMs are helpful but not perfect
   - Real hardware testing is essential

3. **Keep LTO disabled for legacy builds**
   - Unless you've tested it thoroughly
   - Compatibility > Performance for legacy

4. **Use C++20 features carefully**
   - Some C++20 library features may be limited
   - Test standard library usage

5. **Check API availability**
   - Use `@available` annotations
   - Provide fallbacks for missing APIs

## Support

If you encounter issues:
1. Check the workflow run logs
2. Verify deployment target with `otool`
3. Test on actual hardware running the target macOS
4. Check for newer API usage in code

---

**Quick Start:** Actions → Build macOS Legacy (Manual) → Select deployment target → Run workflow → Download artifact

**Remember:** Always test on actual hardware running your target macOS version!

