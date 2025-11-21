## What's Changed

- add: add support for hashing / encrypting passwords and database with sodium [b434b3e](https://github.com/joshua-9402/cpet-140-final-project/commit/b434b3e22d16f9c56317f68d3afe9f8a10c4a0eb)
- feat: addition of a system vault for storing passwords securely [c1e9b30](https://github.com/joshua-9402/cpet-140-final-project/commit/c1e9b307e446713a1423e2ea032b66c017d8a8b1)
- fix: fix error in GitHub Actions workflow for building releases [0f6adc9](https://github.com/joshua-9402/cpet-140-final-project/commit/0f6adc980e39a2cf8797054add2651cb09d7eac9)
- refactor: improve code accessibility and consistency in all module [f7b9c6a](https://github.com/joshua-9402/cpet-140-final-project/commit/f7b9c6ab36af64b11a7b6f08dac7ed60a04e4e0c)
- remove: remove all function related to text files as database storage [fa8e444](https://github.com/joshua-9402/cpet-140-final-project/commit/fa8e4445af642b4cf70ab2d0bf3702dc646d93a3)

## Supported Platforms

### Linux
- **Debian/Ubuntu**: Ubuntu 22.04, 20.04, Debian 12 (x64, ARM64)
- **Red Hat/Fedora**: Fedora Latest, AlmaLinux 9, Rocky Linux 9 (x64, ARM64)
- **SUSE**: openSUSE Tumbleweed, Leap 15.6 (x64, ARM64)

### macOS
- Apple Silicon (ARM64) - macOS 11.0+
- Intel (x64) - macOS 10.15+

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
