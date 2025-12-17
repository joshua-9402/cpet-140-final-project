## What's Changed

### Cross-platform libsodium auto-install & installer improvements
- Implemented automatic libsodium installation for all platforms:
  - Linux: `packaging/linux/postinst.sh` (post-install script hooked into .deb/.rpm)
  - Windows: `packaging/windows/install_libsodium.ps1` (PowerShell) and NSIS integration
  - macOS: `packaging/macos/postinstall` (postinstall script for .pkg)
- Made `assets/icons/app_icon.png` the default application icon (window/dock/taskbar) and added a macOS helper to set Dock icon at runtime.
- Added NSIS installer commands to create a desktop shortcut on Windows.
- Bundled logic in `CMakeLists.txt` to copy the icon asset, include platform-specific helper sources, and include the post-install scripts in generated packages.
- Added documentation and guides:
  - `doc/LINUX_LIBSODIUM_AUTO_INSTALL.md`
  - `doc/LINUX_AUTO_INSTALL_SUMMARY.md`
  - `doc/LINUX_AUTO_INSTALL_QUICKREF.md`
  - `doc/CROSS_PLATFORM_AUTO_INSTALL.md`
  - `doc/AUTO_INSTALL_COMPLETE.md`
  - Updated `doc/LIBSODIUM_PREBUILT_GUIDE.md`


## Supported Platforms

### Linux
- **Debian/Ubuntu**: Ubuntu 22.04, 20.04, Debian 12 (x64, ARM64)
- **Red Hat/Fedora**: Fedora Latest, AlmaLinux 9, Rocky Linux 9 (x64, ARM64)
- **SUSE**: openSUSE Tumbleweed, Leap 15.6 (x64, ARM64)

### macOS
- Apple Silicon (ARM64) - macOS 11.0+
- Intel (x64) - macOS 15.0+

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

For issues, questions, or feature requests, please visit the [Issues](
https://github.com/joshua-9402/cpet-140-final-project/issues) page.

## Documentation

See the [README](README.md) for full documentation and usage instructions.
