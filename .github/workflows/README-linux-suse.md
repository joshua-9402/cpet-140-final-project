# Linux Build Workflows (SUSE Family)

## Overview

This workflow builds the **structuracost** application for **SUSE family distributions** across **x64 and ARM64 architectures**.

## Supported Configurations

### SUSE Family Distributions
- **openSUSE Tumbleweed** - Rolling release (bleeding edge)
- **openSUSE Leap 15.6** - Latest stable release
- **openSUSE Leap 15.5** - Previous stable release
- **openSUSE Leap 15.4** - Older stable release

### Compatible Distributions
These SUSE-based distributions should also work with the builds:
- **SUSE Linux Enterprise (SLE)** - Commercial (use openSUSE Leap builds)
- **GeckoLinux** - Based on openSUSE
- **Aeon** - Immutable desktop based on openSUSE Tumbleweed
- **MicroOS** - Immutable server based on openSUSE

### Architectures
- **x64** (AMD64/Intel 64-bit)
- **ARM64** (AArch64/ARM 64-bit)

### Total Combinations
**4 distributions × 2 architectures = 8 possible builds**

**Plus compatibility with SUSE Linux Enterprise and derivatives!**

## Key Features

- ✅ **Manual trigger only** - Build when you need it
- ✅ **Multi-version support** - Tumbleweed (rolling) and Leap (stable)
- ✅ **Multi-architecture** - x64 and ARM64
- ✅ **Container-based builds** - Consistent environment
- ✅ **Zypper package manager** - SUSE standard
- ✅ **Configurable** - Choose Release or Debug
- ✅ **Optional LTO** - Enable/disable Link-Time Optimization
- ✅ **90-day retention** - Long artifact storage
- ✅ **SLE-compatible** - openSUSE Leap matches SLE versions

## How to Trigger a Build

### From GitHub Website

1. Go to your repository on GitHub
2. Click the **Actions** tab
3. Select **"Linux (SUSE Family)"** from the workflow list
4. Click the **"Run workflow"** button
5. Configure options:
   - **Distribution:** Choose your distribution
     - `opensuse-tumbleweed` - Rolling release (latest packages)
     - `opensuse-leap-15.6` - Leap 15.6 (stable)
     - `opensuse-leap-15.5` - Leap 15.5
     - `opensuse-leap-15.4` - Leap 15.4
   - **Architecture:** Choose `x64` or `arm64`
   - **Build type:** Choose `Release` or `Debug`
   - **Enable LTO:** Check for maximum performance
6. Click **"Run workflow"**

### From GitHub CLI

```bash
# openSUSE Tumbleweed x64 Release
gh workflow run "Linux (SUSE Family)" \
  --ref master \
  -f distribution=opensuse-tumbleweed \
  -f architecture=x64 \
  -f build_type=Release \
  -f enable_lto=true

# openSUSE Leap 15.6 ARM64 Release
gh workflow run "Linux (SUSE Family)" \
  --ref master \
  -f distribution=opensuse-leap-15.6 \
  -f architecture=arm64 \
  -f build_type=Release \
  -f enable_lto=true

# openSUSE Leap 15.5 x64 Debug
gh workflow run "Linux (SUSE Family)" \
  --ref master \
  -f distribution=opensuse-leap-15.5 \
  -f architecture=x64 \
  -f build_type=Debug \
  -f enable_lto=false
```

## Distribution Details

### openSUSE Tumbleweed (Rolling Release)

**Package Manager:** zypper  
**Compiler:** GCC (latest stable)  
**Best For:** Desktop users who want latest packages, developers

**Version:**
- `opensuse-tumbleweed` - Rolling release (always current)

**Release Cycle:** Continuous (snapshots tested daily)  
**Support:** Continuous updates

**What is Tumbleweed?**
- Rolling release distribution
- Always has the latest software
- Tested before release (not "unstable")
- Great for desktops and development
- Automatic snapshots with Btrfs

### openSUSE Leap (Stable Release)

**Package Manager:** zypper  
**Compiler:** GCC (stable version)  
**Best For:** Servers, enterprise, stability-focused users

**Versions:**
- `opensuse-leap-15.6` - Latest Leap (based on SLE 15 SP6)
- `opensuse-leap-15.5` - Previous version (based on SLE 15 SP5)
- `opensuse-leap-15.4` - Older version (based on SLE 15 SP4)

**Release Cycle:** ~1 year between minor versions  
**Support:** ~18 months per minor version

**What is Leap?**
- Stable release distribution
- Based on SUSE Linux Enterprise (SLE)
- Binary compatible with SLE
- Perfect for servers and enterprise
- Long-term stability

### SUSE Linux Enterprise (SLE)

**Commercial distribution** (requires subscription)

Use **openSUSE Leap** builds for SLE compatibility:
- SLE 15 SP6 → Use openSUSE Leap 15.6 build
- SLE 15 SP5 → Use openSUSE Leap 15.5 build
- SLE 15 SP4 → Use openSUSE Leap 15.4 build

Binary compatible due to shared source!

### Why SUSE Family?

**Quality:**
- Rigorous testing (especially Tumbleweed)
- Professional development
- Strong quality assurance

**Enterprise Support:**
- Leap is based on SLE
- Free version of enterprise distribution
- Great for business use

**YaST:**
- Powerful system configuration tool
- Makes administration easier
- Comprehensive management

**Btrfs by default:**
- Automatic snapshots
- Easy rollback if updates cause issues
- Advanced filesystem features

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
**Common on:** ARM servers, some development boards

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
3. Download: `payroll-and-monitoring-system-opensuse-tumbleweed-x64-Release.tar.gz`

**Retention:** 90 days

### Extract and Run

```bash
# Extract
tar -xzf payroll-and-monitoring-system-opensuse-tumbleweed-x64-Release.tar.gz

# Run
./payroll-and-monitoring-system
```

### Check Dependencies

```bash
# See required libraries
ldd payroll-and-monitoring-system

# Common dependencies:
# - libfreetype.so.6
# - libGL.so.1
# - libX11.so.6
# - libstdc++.so.6
# - libgcc_s.so.1
```

## Compatibility Matrix

| Distribution | Architecture | Tested On | Likely Compatible With |
|--------------|--------------|-----------|------------------------|
| Tumbleweed | x64 | openSUSE Tumbleweed | openSUSE Tumbleweed, GeckoLinux Rolling |
| Tumbleweed | arm64 | openSUSE Tumbleweed ARM64 | openSUSE ARM, MicroOS |
| Leap 15.6 | x64 | openSUSE Leap 15.6 | SLE 15 SP6, Leap 15.6+ |
| Leap 15.6 | arm64 | openSUSE Leap 15.6 ARM64 | SLE 15 SP6 ARM, Leap 15.6+ ARM |
| Leap 15.5 | x64 | openSUSE Leap 15.5 | SLE 15 SP5, Leap 15.5+ |
| Leap 15.4 | x64 | openSUSE Leap 15.4 | SLE 15 SP4, Leap 15.4+ |

## System Requirements (For Running)

**All Distributions:**
- OpenGL support (hardware or software)
- X11 display server
- Required shared libraries (installed via package manager)

**Installing Dependencies (End User):**

**openSUSE (all versions):**
```bash
sudo zypper install freetype2 Mesa-libGL libX11-6 libXrandr2 libXinerama1 libXcursor1 libXi6
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

### openSUSE Tumbleweed

- **Rolling release** - always up-to-date
- Tested snapshots before release
- Automatic Btrfs snapshots for rollback
- Latest GCC and development tools
- Great for development

**Use when:**
- You want the latest software
- Desktop usage
- Development work
- Don't mind occasional updates

### openSUSE Leap

- **Stable release** - predictable
- Based on SUSE Linux Enterprise
- Binary compatible with SLE
- Longer support lifecycle
- Perfect for servers

**Use when:**
- Need stability
- Server deployments
- Enterprise environments
- Don't need bleeding edge

### Leap vs Tumbleweed

| Feature | Tumbleweed | Leap |
|---------|-----------|------|
| **Release Model** | Rolling | Fixed |
| **Updates** | Continuous | Point releases |
| **Stability** | Very good | Excellent |
| **Latest Software** | Yes | No |
| **Best For** | Desktop, dev | Server, enterprise |
| **SLE Compatible** | No | Yes |

## Which Distribution to Choose?

**For end users (desktop):**
- Use openSUSE Tumbleweed x64 (latest software)

**For developers:**
- Use openSUSE Tumbleweed x64 (latest tools)

**For servers (production):**
- Use openSUSE Leap 15.6 x64 (stable, SLE-compatible)

**For SUSE Linux Enterprise compatibility:**
- Match Leap version to SLE version
  - SLE 15 SP6 → Leap 15.6
  - SLE 15 SP5 → Leap 15.5

**For ARM servers:**
- Use openSUSE Leap 15.6 arm64 or Tumbleweed arm64

## ARM64 Considerations

**Performance:**
- Native ARM64 build performs well on ARM hardware
- May use emulation if ARM64 runners unavailable

**Compatibility:**
- ARM servers running openSUSE
- Some development boards with openSUSE ARM

**Limitations:**
- Fewer ARM64 runners available
- May use QEMU emulation (slower)
- Some packages may not be available

## Best Practices

1. **Choose the right distribution:**
   - Tumbleweed for latest software
   - Leap for stability
   - Match your deployment target

2. **Test on target distribution:**
   - Don't assume cross-distribution compatibility
   - Verify dependencies are available

3. **Use Leap for production servers:**
   - More stable
   - Longer support period
   - SLE compatibility

4. **Include installation instructions:**
   - List required dependencies
   - Provide zypper install commands

## Performance Comparison

| Distribution | Build Speed | Binary Size | Optimization | Updates |
|--------------|-------------|-------------|--------------|---------|
| Tumbleweed | Fast | Medium | Excellent | Continuous |
| Leap 15.6 | Fast | Medium | Good | Point releases |
| Leap 15.5 | Fast | Medium | Good | Point releases |
| Leap 15.4 | Medium | Medium | Good | Point releases |

## YaST and System Management

**YaST** (Yet another Setup Tool) is SUSE's system configuration tool:
- Comprehensive system management
- Easy package installation
- Network configuration
- User management
- And much more

Installing dependencies with YaST:
```bash
sudo yast2 sw_single
# Search for: freetype2, Mesa-libGL, libX11-6, etc.
```

## Btrfs Snapshots

SUSE uses Btrfs by default with automatic snapshots:
- Snapshots created before updates
- Easy rollback if something breaks
- Boot into previous snapshot
- System remains stable

If an update causes issues, simply boot into a previous snapshot!

## Support

If you encounter issues:
1. Check the workflow run logs
2. Verify distribution and architecture
3. Check dependency installation logs
4. Test on actual target system
5. Check openSUSE forums and documentation

---

**Quick Start:** Actions → Linux (SUSE Family) → Select distribution + architecture → Run workflow

**Remember:** Test the binary on your actual target distribution and architecture!

## Additional Resources

- **openSUSE Website:** https://www.opensuse.org/
- **openSUSE Wiki:** https://en.opensuse.org/
- **SUSE Documentation:** https://documentation.suse.com/
- **openSUSE Forums:** https://forums.opensuse.org/

