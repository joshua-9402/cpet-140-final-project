# StructuraCost - Windows Installation Guide

## Installation

1. **Run the installer** (`structuracost-setup.exe`)
2. **Follow the installation wizard**
3. **Wait for automatic dependency installation**
   - The installer will automatically download and install:
     ✓ Visual C++ Redistributable 2015-2022 (x64)
     ✓ Visual C++ Redistributable 2015-2022 (x86)
     ✓ libsodium.dll (required encryption library)
   - This may take a few minutes
   - A PowerShell window will show installation progress
4. **Click Finish**

The installer creates:
- Installation folder: `C:\Program Files\StructuraCost\`
- Desktop shortcut
- Start menu entry

## First Time Launch

**Just double-click the desktop shortcut or start menu item!**

All required dependencies are now installed automatically.

## If you are running a ZIP build / copied EXE (important)

If you copy `structuracost.exe` to another computer, it **must** be a **Release** build.

- If Windows says **`ucrtbased.dll`** or **`vcruntime140d.dll`** is missing, that EXE was built against the **Debug** MSVC runtime and is not distributable.
- Installing the VC++ Redistributable will **not** fix `...d.dll` missing errors.

### Quick verification (recommended before distributing)

If you have Visual Studio Build Tools installed (so `dumpbin` exists), run:

- `verify_release_build.ps1` (included in this repo):

  powershell -ExecutionPolicy Bypass -File "packaging\\windows\\verify_release_build.ps1" -ExecutablePath "path\\to\\structuracost.exe"

The script will fail if it detects forbidden debug DLL dependencies.

## What If Installation Fails?

If the automatic dependency installation fails, you'll see a message with manual installation instructions.

### Manual Installation (Rare):

**If error 0xc000007b appears:**

Download and install BOTH:
1. [VC++ x64](https://aka.ms/vs/17/release/vc_redist.x64.exe)
2. [VC++ x86](https://aka.ms/vs/17/release/vc_redist.x86.exe)

Then **restart your computer** and try again.

### If libsodium installation failed:

1. Download: https://download.libsodium.org/libsodium/releases/libsodium-1.0.20-stable-msvc.zip
2. Extract the ZIP file
3. Copy `libsodium.dll` from:
   - **64-bit Windows**: `x64\Release\v143\dynamic\libsodium.dll`
   - **32-bit Windows**: `Win32\Release\v143\dynamic\libsodium.dll`
4. Paste it into: `C:\Program Files\StructuraCost\`

## Running the Application

1. **Double-click** the desktop shortcut, OR
2. Navigate to `C:\Program Files\StructuraCost\` and run `structuracost.exe`

## System Requirements

- **OS**: Windows 10 or later (64-bit recommended)
- **RAM**: 4 GB minimum, 8 GB recommended
- **Storage**: 500 MB available space
- **Dependencies**:
  - Visual C++ Redistributable 2015-2022
  - libsodium (installed automatically)

## Troubleshooting

### Application won't start
- Install Visual C++ Redistributables (see links above)
- Restart computer
- Run as administrator (right-click → Run as administrator)

### Missing DLL errors
- Reinstall Visual C++ Redistributables
- Manually install libsodium.dll (see above)

### Full troubleshooting guide
See: `doc\WINDOWS_ERROR_0xc000007b_FIX.md` in installation directory

## Uninstallation

1. **Windows Settings** → Apps → StructuraCost → Uninstall, OR
2. **Control Panel** → Programs → Uninstall a program → StructuraCost

## Data Location

Your data is stored in:
```
C:\Users\<YourUsername>\AppData\Local\StructuraCost\data\
```

**Backup this folder** before uninstalling to preserve your data!

## Support

- Documentation: Check the `doc\` folder in the installation directory
- Issues: Report on GitHub
- Logs: `C:\Users\<YourUsername>\AppData\Local\StructuraCost\logs\`

---

**Version:** 1.0.0
**Last Updated:** December 18, 2024
