# Quick CI/CD Reference

## What Gets Built Automatically?

Every push to `main`, `master`, or `develop` branches triggers builds for:
- Linux (Ubuntu, x64)
- macOS (Intel x64 + Apple Silicon ARM64)
- Windows (x64)

## Where to Find Build Artifacts?

### During Development
1. Go to: `https://github.com/YOUR_USERNAME/YOUR_REPO/actions`
2. Click the latest workflow run
3. Download artifacts from the bottom of the page (available for 30 days)

### For Releases
Create a git tag starting with `v`:
```bash
git tag v1.0.0
git push origin v1.0.0
```

This creates a GitHub Release with all platform builds attached permanently.

## File Structure of Artifacts

### Linux/macOS (.tar.gz)
```
payroll-and-monitoring-system     # executable
assets/                           # required assets folder
  fonts/
  icons/
```

### Windows (.zip)
```
payroll-and-monitoring-system.exe # executable
assets/                           # required assets folder
  fonts/
  icons/
```

### macOS App Bundle (.tar.gz)
```
payroll-and-monitoring-system.app/
  Contents/
    MacOS/
      payroll-and-monitoring-system
    Resources/
      assets/
```

## Common Issues

### Build Failed?
- Check the Actions tab for error logs
- Verify CMakeLists.txt has no syntax errors
- Ensure all source files are committed

### Can't Find Artifacts?
- Wait for the workflow to complete (green checkmark)
- Artifacts appear at the bottom of the workflow run page
- They expire after 30 days (use releases for permanent storage)

### Want to Build Only One Platform?
You can disable matrix builds by commenting out unwanted configs in `.github/workflows/build.yml`

## Modifying the Workflow

Location: `.github/workflows/build.yml`

### Add a New Platform
```yaml
- {
    name: "Platform Name",
    os: runner-os,           # ubuntu-latest, macos-latest, windows-latest
    arch: architecture,      # x64, arm64
    artifact: "artifact-name",
    build_type: "Release"
  }
```

### Add Dependencies
Add installation commands in the appropriate section:
- `Install Linux dependencies` for apt packages
- `Install macOS dependencies` for brew packages
- `Install Windows dependencies` for choco packages

## Manual Local Builds Matching CI

### Linux/macOS
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build build --config Release -j 4
```

### Windows
```cmd
cmake -B build -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build build --config Release -j 4
```

### macOS (specific architecture)
```bash
# Apple Silicon
cmake -B build -DCMAKE_BUILD_TYPE=Release -G Ninja -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build --config Release -j 4

# Intel
cmake -B build -DCMAKE_BUILD_TYPE=Release -G Ninja -DCMAKE_OSX_ARCHITECTURES=x86_64
cmake --build build --config Release -j 4
```

## Tips

- Artifacts are named with platform and architecture for easy identification
- Release builds have optimizations enabled (-O3 for GCC/Clang, /O2 for MSVC)
- All builds include the `assets/` folder automatically
- Workflow runs in parallel across all platforms (faster total build time)

