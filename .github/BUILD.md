# CI/CD Build Documentation

## Overview
This project uses GitHub Actions to automatically build the application for multiple platforms and architectures whenever code is pushed to the repository.

## Supported Platforms

The CI/CD pipeline builds for the following platforms:

| Platform | Architecture | Artifact Name |
|----------|-------------|---------------|
| Ubuntu Linux | x64 | `payroll-and-monitoring-system-linux-x64.tar.gz` |
| macOS | x64 (Intel) | `payroll-and-monitoring-system-macos-x64.tar.gz` |
| macOS | ARM64 (Apple Silicon) | `payroll-and-monitoring-system-macos-arm64.tar.gz` |
| Windows | x64 | `payroll-and-monitoring-system-windows-x64.zip` |

## How It Works

### Automatic Builds
The workflow automatically triggers on:
- Push to `main`, `master`, or `develop` branches
- Pull requests to `main`, `master`, or `develop` branches
- Manual workflow dispatch (you can trigger it manually from GitHub Actions tab)

### Build Process
For each platform, the workflow:
1. Checks out the code
2. Installs platform-specific dependencies
3. Configures CMake with Release build type
4. Compiles the application
5. Packages the executable with assets
6. Uploads artifacts (available for 30 days)

### Dependencies Installed
- **Linux**: CMake, Ninja, OpenGL, FreeType, X11 development libraries
- **macOS**: CMake, Ninja, FreeType (via Homebrew)
- **Windows**: CMake, Ninja (via Chocolatey)

## Downloading Build Artifacts

### From GitHub Actions
1. Go to the **Actions** tab in your GitHub repository
2. Click on the latest workflow run
3. Scroll to the **Artifacts** section at the bottom
4. Download the artifact for your platform

### From Releases (Tagged Versions)
When you create a git tag starting with `v` (e.g., `v1.0.0`):
```bash
git tag v1.0.0
git push origin v1.0.0
```

The workflow will automatically create a GitHub Release with all platform builds attached.

## Manual Workflow Trigger

You can manually trigger a build from GitHub:
1. Go to **Actions** tab
2. Select **Multi-Platform Build** workflow
3. Click **Run workflow**
4. Select the branch and click **Run workflow**

## Local Build Instructions

### Prerequisites
Ensure you have:
- CMake 4.0+
- C++23 compatible compiler
- Ninja build system (recommended) or Make
- Platform-specific dependencies (see above)

### Build Commands

#### Linux/macOS
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build build --config Release -j 4
```

#### macOS (specify architecture)
```bash
# For Apple Silicon (ARM64)
cmake -B build -DCMAKE_BUILD_TYPE=Release -G Ninja -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build --config Release -j 4

# For Intel (x64)
cmake -B build -DCMAKE_BUILD_TYPE=Release -G Ninja -DCMAKE_OSX_ARCHITECTURES=x86_64
cmake --build build --config Release -j 4
```

#### Windows
```cmd
cmake -B build -DCMAKE_BUILD_TYPE=Release -G Ninja
cmake --build build --config Release -j 4
```

## Optimizations

The CMakeLists.txt includes platform-specific optimizations:
- **GCC/Clang**: `-O3 -march=native -flto` (Link-Time Optimization)
- **MSVC**: `/O2 /GL /LTCG` (Maximum optimization + Link-Time Code Generation)

Note: `-march=native` is disabled in CI builds for broader compatibility.

## Troubleshooting

### Build Failures
- Check the Actions tab for detailed error logs
- Ensure all dependencies are properly declared in CMakeLists.txt
- Verify that assets folder structure matches the application expectations

### Asset Loading Issues
The application looks for assets in:
- Current working directory
- Executable directory
- macOS: `.app/Contents/Resources/assets`

Make sure to distribute the `assets/` folder with your executable.

## Contributing
When adding new dependencies or build requirements:
1. Update CMakeLists.txt
2. Update the workflow file (`.github/workflows/build.yml`) to install new dependencies
3. Test locally on multiple platforms if possible
4. Update this documentation

## Notes
- Artifacts are retained for 30 days by default
- Each build runs in parallel, reducing total build time
- The workflow uses Ninja for faster builds compared to Make/MSBuild
- Cross-compilation is not used; each platform builds natively

