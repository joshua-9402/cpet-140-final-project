# Release Mode Guide

This guide explains how to use the `mode` option in the release workflow to either create a release or test the build process.

## Overview

The release workflow now supports two modes:
- **`release`** - Builds all artifacts and creates a GitHub release
- **`test`** - Builds all artifacts but does NOT create a release (for testing purposes)

## How to Use

### Test Mode (Recommended First)

Before creating an actual release, it's recommended to test the build process first:

1. Go to **Actions** → **Release** workflow
2. Click **Run workflow**
3. Select **mode**: `test`
4. Configure version numbers and platforms as desired
5. Click **Run workflow**

**What happens in test mode:**
- All selected platforms will be built (Linux Debian, RPM, SUSE, macOS, Windows)
- Build artifacts are uploaded and available for 7 days
- A summary is displayed showing:
  - Version number
  - Build results for each platform
  - List of all generated files
- **NO GitHub release is created**
- No git tags are created

**When to use test mode:**
- Testing the build configuration
- Verifying all platforms build successfully
- Checking artifact names and sizes
- Validating the build before an official release

### Release Mode

Once you've verified the build works in test mode, you can create an actual release:

1. Go to **Actions** → **Release** workflow
2. Click **Run workflow**
3. Select **mode**: `release`
4. Configure version numbers and platforms
5. Click **Run workflow**

**What happens in release mode:**
- All selected platforms will be built
- Build artifacts are collected
- A GitHub release is created with:
  - Release notes from CHANGELOG.md
  - All build artifacts (installers and archives)
  - Appropriate pre-release flag based on stage
  - Git tag matching the version

## Release vs Test

- Release mode will create a GitHub Release when the CI job is configured to do so and upload artifacts there.
- Test mode runs identical build steps but only uploads artifacts to the workflow run (no release created). Use Test to verify builds before publishing.

## CI libsodium reminder
Ensure libsodium is installed/provided prior to CMake configure in CI when running Release or Test builds. See `doc/LIBSODIUM_SETUP.md`.

---

## Repository sync note
Updated: 2025-11-29

## Version Examples

### Test a Beta Build
- **mode**: `test`
- **version_w**: `1`
- **version_x**: `beta`
- **version_y**: `0`
- **version_z**: `0`
- **stage_iteration**: `1`

Result: Builds `v1.2.0.0-beta.1` without creating a release

### Release a Stable Version
- **mode**: `release`
- **version_w**: `1`
- **version_x**: `stable`
- **version_y**: `0`
- **version_z**: `0`
- **stage_iteration**: `0`

Result: Builds and releases `v1.4.0.0` (X=4 for stable, no suffix)

### Release a Release Candidate
- **mode**: `release`
- **version_w**: `2`
- **version_x**: `rc`
- **version_y**: `1`
- **version_z**: `5`
- **stage_iteration**: `2`
Result: Builds and releases `v1.4.0.0`
## Artifact Retention

- Test mode artifacts are kept for **7 days**
- You can download them from the workflow run page
- After 7 days, they are automatically deleted (no release is preserved)

## Build Artifacts Detail

Each build job produces specific artifacts:

### Linux Debian/Ubuntu Build
**Job**: `build-linux-debian`
- **Platform**: Ubuntu 22.04 (x64 or arm64)
- **Artifacts**:
  - `.deb` installer package
  - `-archive.tar.gz` portable archive
- **Naming**: `structuracost-{version}-linux-{arch}.deb`

### Linux RPM Build (Fedora/RHEL/CentOS)
**Job**: `build-linux-rpm`
- **Platform**: Fedora 40 (x64 or arm64)
- **Artifacts**:
  - `.rpm` installer package
  - `-archive.tar.gz` portable archive
- **Naming**: `structuracost-{version}-linux-{arch}.rpm`
## Workflow Structure

```
prepare-version
    ↓
[build-linux-debian, build-linux-rpm, build-linux-suse, build-macos, build-windows]
    ↓
create-release (if mode == 'release')
    OR
test-summary (if mode == 'test')
```

Both `create-release` and `test-summary` will download all artifacts, but only `create-release` will create a GitHub release.
