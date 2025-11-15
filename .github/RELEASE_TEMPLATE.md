## What's Changed

- feat: creates an installer for every supported platform [7878e27](https://github.com/joshua-9402/cpet-140-final-project/commit/7878e27508954b9354607b59fb0729cf2b10fa2e)
- feat: adds Linux desktop integration (.desktop file, icon, post-install cache updates) [49284a4](https://github.com/joshua-9402/cpet-140-final-project/commit/49284a476d2fcaef9ec57271a2151ca1b006ea6c)
- feat: adds macOS bundle identifiers (com.structuracost.app) [49284a4](https://github.com/joshua-9402/cpet-140-final-project/commit/49284a476d2fcaef9ec57271a2151ca1b006ea6c)
- feat: adds Windows desktop shortcuts and Start Menu entries for both NSIS and WiX installers [49284a4](https://github.com/joshua-9402/cpet-140-final-project/commit/49284a476d2fcaef9ec57271a2151ca1b006ea6c)
- feat: adds platform-native metadata (bundle IDs, WiX UpgradeCode, package descriptions) [49284a4](https://github.com/joshua-9402/cpet-140-final-project/commit/49284a476d2fcaef9ec57271a2151ca1b006ea6c)
- feat: add release classification labels [f68ee3e](https://github.com/joshua-9402/cpet-140-final-project/commit/f68ee3e09ccd8d9d6cf60a2c19b521fc9cb89f12)

## Supported Platforms

### Linux
- **Debian/Ubuntu**: Ubuntu 22.04, 20.04, Debian 12 (x64, ARM64)
- **Red Hat/Fedora**: Fedora Latest, AlmaLinux 9, Rocky Linux 9 (x64, ARM64)
- **SUSE**: openSUSE Tumbleweed, Leap 15.6 (x64, ARM64)

### macOS
- Apple Silicon (ARM64) - macOS 14.0+
- Intel (x64) - macOS 11.0+

### Windows
- x64 and ARM64


## Build Information
- Build Type: Pre-Release
- Optimizations: Enabled (LTO)
- Compiler Flags: -O3 (Linux/macOS), /O2 (Windows)

## Requirements

### Linux
All builds require standard system libraries:
```bash
# Debian/Ubuntu
sudo apt-get install libfreetype6 libgl1 libx11-6 libxrandr2 libxinerama1 libxcursor1 libxi6


# Fedora/Red Hat
sudo dnf install freetype mesa-libGL libX11 libXrandr libXinerama libXcursor libXi

# SUSE
sudo zypper install freetype2 Mesa-libGL libX11-6 libXrandr2 libXinerama1 libXcursor1 libXi6
```

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

See the [README](../README.md) for full documentation and usage instructions.


