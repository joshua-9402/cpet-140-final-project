## What's Changed

- Major rewrite of project structure and build systems
  - Simplified build targets and added explicit installation rules, exported targets, and package config.
  - CI pipeline updated to build, test, and produce distributable artifacts for supported platforms.
  - Result: clearer module boundaries, faster incremental builds, and reproducible artifacts.

- Removed Linux support:
  - Linux-specific binaries and CI were removed to focus maintenance on macOS and Windows.
  - Rationale: limited maintenance resources and priority on native macOS/Windows integrations.
  - Impact: no prebuilt Linux packages or CI verification; Linux users must build from source and may need to adapt platform-specific code.
  - Migration: if you depend on Linux, consider forking prior to this change or restoring Linux-specific commits from the repository history.
  - Note: This is a breaking change for users/projects deploying on Linux; update deployment scripts and packaging accordingly.


## Supported Platforms

### macOS
- Apple Silicon (ARM64) - macOS 11.0+
- Intel (x64) - macOS 15.0+

### Windows
- x64 and ARM64


## Build Information
- Build Type: Release
- Optimizations: Enabled (LTO)
- Compiler Flags: -O3 (macOS), /O2 (Windows)

## Requirements

### macOS
You need to execute this in the terminal before launching the app as macOS shows this error because the app is not code-signed.
macOS Gatekeeper blocks unsigned apps downloaded from the internet by setting a "quarantine" attribute.
```bash
xattr -cr <location-of-the-.app>
```

### Windows
No additional dependencies required.

## Support

For issues, questions, or feature requests, please visit the [Issues](https://github.com/joshua-9402/cpet-140-final-project/issues) page.

## Documentation

See the [README](README.md) for full documentation and usage instructions.
