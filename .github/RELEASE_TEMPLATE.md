## What's Changed

- feat: creates an installer for every supported platform [7878e27](https://github.com/joshua-9402/cpet-140-final-project/commit/7878e27508954b9354607b59fb0729cf2b10fa2e)
- feat: adds Linux desktop integration (.desktop file, icon, post-install cache updates) [49284a4](https://github.com/joshua-9402/cpet-140-final-project/commit/49284a476d2fcaef9ec57271a2151ca1b006ea6c)
- feat: adds macOS bundle identifiers (com.structuracost.app) [49284a4](https://github.com/joshua-9402/cpet-140-final-project/commit/49284a476d2fcaef9ec57271a2151ca1b006ea6c)
- feat: adds Windows desktop shortcuts and Start Menu entries for both NSIS and WiX installers [49284a4](https://github.com/joshua-9402/cpet-140-final-project/commit/49284a476d2fcaef9ec57271a2151ca1b006ea6c)
- feat: adds platform-native metadata (bundle IDs, WiX UpgradeCode, package descriptions) [49284a4](https://github.com/joshua-9402/cpet-140-final-project/commit/49284a476d2fcaef9ec57271a2151ca1b006ea6c)

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

## Installation

### Installers & Artifacts

Choose the installer matching your OS and architecture:

**Linux:**
- Debian/Ubuntu: `structuracost-linux-debian-{arch}-{version}.deb`
- Fedora/Red Hat: `structuracost-linux-rpm-{arch}-{version}.rpm`
- SUSE: `structuracost-linux-suse-{arch}-{version}.rpm`

**macOS:**
- Apple Silicon: `structuracost-macos-arm64-{version}.dmg` or `.pkg`
- Intel: `structuracost-macos-intel-{version}.dmg` or `.pkg`

**Windows:**
- x64: `structuracost-windows-x86_64-{version}.exe` or `.msi`
- ARM64: `structuracost-windows-arm64-{version}.exe` or `.msi`

**Fallback archives** (`.tar.gz` for Linux/macOS, `.zip` for Windows) are also provided.

---

### Linux

**Using Package Managers (Recommended):**

```bash
# Debian/Ubuntu
sudo apt install ./structuracost-linux-debian-x86_64-{version}.deb

# Fedora/AlmaLinux/Rocky Linux
sudo dnf install ./structuracost-linux-rpm-x86_64-{version}.rpm

# openSUSE/SUSE
sudo zypper install ./structuracost-linux-suse-x86_64-{version}.rpm
```

**Manual Installation (Fallback):**

```bash
# Extract archive
tar -xzf structuracost-linux-*.tar.gz

# Run directly
./structuracost

# Or install system-wide (optional)
sudo mkdir -p /opt/structuracost
sudo cp -r * /opt/structuracost/
sudo ln -sf /opt/structuracost/bin/structuracost /usr/bin/structuracost
structuracost
```

**Desktop Integration:**
- Package installers automatically create a desktop entry and install the application icon
- Find "StructuraCost" in your application menu under Office/Utilities

---

### macOS

**Using DMG (Recommended):**
1. Download `structuracost-macos-{arch}-{version}.dmg`
2. Open the DMG file
3. Drag StructuraCost.app to Applications folder
4. Launch from Applications or Launchpad

**Using PKG Installer:**
1. Download `structuracost-macos-{arch}-{version}.pkg`
2. Double-click to run the installer
3. Follow the installation wizard
4. Launch from Applications

**Removing Gatekeeper Quarantine (Unsigned Builds):**
```bash
# If macOS blocks the app, remove the quarantine attribute
xattr -cr "/Applications/StructuraCost.app"
```

**Manual Installation (Fallback):**
```bash
tar -xzf structuracost-macos-{arch}-{version}.tar.gz
open structuracost.app
```

---

### Windows

**Using Installers (Recommended):**

1. Download either:
   - `structuracost-windows-x86_64-{version}.exe` (NSIS installer)
   - `structuracost-windows-x86_64-{version}.msi` (WiX installer)

2. Run the installer and follow the setup wizard

3. Launch StructuraCost from:
   - **Desktop shortcut** (created automatically)
   - **Start Menu** → StructuraCost
   - Search for "StructuraCost" in Windows Search

**Manual Installation (Fallback):**

```powershell
# Extract archive
Expand-Archive structuracost-windows-x86_64-{version}.zip -DestinationPath .\structuracost

# Navigate to folder
cd structuracost

# Run the application
.\structuracost.exe
```

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
```

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


