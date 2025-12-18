# Windows Error 0xc000007b Fix Guide

## Problem Description
Error 0xc000007b occurs when the application fails to start on Windows, typically showing:
> "The application was unable to start correctly (0xc000007b). Click OK to close the application."

## Root Causes
1. **Architecture Mismatch**: Mixing 32-bit and 64-bit DLLs
2. **Missing Runtime Libraries**: Required DLL files not found
3. **Corrupted DLLs**: Damaged or incompatible library files
4. **Visual C++ Redistributables**: Missing Microsoft Visual C++ runtime

## Solutions

### Solution 1: Install Microsoft Visual C++ Redistributables (REQUIRED)

Download and install both versions (even on 64-bit Windows):

**For 64-bit Windows (install BOTH):**
1. [Visual C++ 2015-2022 Redistributable (x64)](https://aka.ms/vs/17/release/vc_redist.x64.exe)
2. [Visual C++ 2015-2022 Redistributable (x86)](https://aka.ms/vs/17/release/vc_redist.x86.exe)

**For 32-bit Windows:**
1. [Visual C++ 2015-2022 Redistributable (x86)](https://aka.ms/vs/17/release/vc_redist.x86.exe)

**Steps:**
1. Download both installers
2. Run `vc_redist.x64.exe` first (on 64-bit systems)
3. Run `vc_redist.x86.exe`
4. Restart your computer
5. Try running StructuraCost again

### Solution 2: Verify Architecture Match

**Check your Windows version:**
1. Press `Win + Pause/Break` or right-click "This PC" → Properties
2. Look for "System type"
   - **64-bit operating system** → Use 64-bit (x64) version of StructuraCost
   - **32-bit operating system** → Use 32-bit (x86) version of StructuraCost

**Ensure you downloaded the correct version:**
- File name should indicate architecture (e.g., `structuracost-x64.exe` or `structuracost-Win32.exe`)
- 64-bit Windows can run 32-bit apps, but NOT vice versa

### Solution 3: Install libsodium Manually

If the automatic installer failed:

1. **Download libsodium:**
   - Visit: https://download.libsodium.org/libsodium/releases/
   - Download: `libsodium-1.0.20-stable-msvc.zip` (or latest version)

2. **Extract the correct DLL:**
   - For 64-bit: Extract from `x64/Release/v143/dynamic/libsodium.dll`
   - For 32-bit: Extract from `Win32/Release/v143/dynamic/libsodium.dll`

3. **Copy to installation directory:**
   - Default: `C:\Program Files\StructuraCost\`
   - Or wherever you installed StructuraCost
   - Place `libsodium.dll` in the same folder as `structuracost.exe`

### Solution 4: Run as Administrator

Sometimes permissions issues cause this error:

1. Right-click `structuracost.exe`
2. Select "Run as administrator"
3. If this works, set it permanently:
   - Right-click → Properties
   - Compatibility tab
   - Check "Run this program as an administrator"
   - Click OK

### Solution 5: Repair Windows System Files

Corrupted system files can cause DLL loading issues:

1. Open Command Prompt as Administrator
2. Run: `sfc /scannow`
3. Wait for scan to complete (may take 15-30 minutes)
4. Restart computer
5. Try running StructuraCost again

### Solution 6: Use Dependency Walker to Diagnose

**For advanced users:**

1. Download [Dependency Walker](http://www.dependencywalker.com/)
2. Open `structuracost.exe` in Dependency Walker
3. Look for:
   - Red icons = missing DLLs
   - Yellow icons = architecture mismatch
   - Note which DLLs are problematic
4. Install/fix the identified missing dependencies

## Quick Checklist

- [ ] Installed Visual C++ Redistributable (x64)
- [ ] Installed Visual C++ Redistributable (x86)
- [ ] Restarted computer after installing redistributables
- [ ] Verified Windows architecture matches app version
- [ ] Confirmed `libsodium.dll` is in the same folder as `structuracost.exe`
- [ ] Tried running as administrator
- [ ] Checked antivirus isn't blocking the application

## Prevention (For Developers/Builders)

### When Building on Windows:

1. **Set correct architecture in CMake:**
   ```bash
   # For 64-bit
   cmake -G "Visual Studio 17 2022" -A x64 ..
   
   # For 32-bit
   cmake -G "Visual Studio 17 2022" -A Win32 ..
   ```

2. **Ensure consistent runtime library:**
   - CMakeLists.txt now sets `MSVC_RUNTIME_LIBRARY` to MultiThreadedDLL
   - All dependencies must use the same runtime

3. **Verify DLL architecture:**
   ```bash
   dumpbin /headers libsodium.dll | findstr machine
   ```
   - Should show: `8664 machine (x64)` for 64-bit
   - Should show: `14C machine (x86)` for 32-bit

4. **Copy all required DLLs:**
   - libsodium.dll (correct architecture)
   - Any other third-party DLLs
   - Place in same directory as executable

## Still Having Issues?

### Create a Diagnostic Report:

1. Open Command Prompt in the StructuraCost installation folder
2. Run:
   ```cmd
   dir *.dll > dll_list.txt
   systeminfo > system_info.txt
   dumpbin /dependents structuracost.exe > dependencies.txt
   ```
3. Attach these files when reporting the issue

### Common Error Variations:

- **0xc000007b**: Architecture or DLL mismatch
- **0xc0000135**: .NET Framework or VC++ runtime missing
- **0xc0000142**: Application initialization failed
- **0xc000001d**: Illegal instruction (wrong CPU architecture)

All these usually resolve with VC++ Redistributables installation.

## Additional Resources

- [Microsoft Visual C++ Downloads](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads)
- [libsodium Official Releases](https://download.libsodium.org/libsodium/releases/)
- [Windows DLL Error Reference](https://support.microsoft.com/en-us/windows)

## Support

If none of these solutions work:
1. Check the application logs in: `%APPDATA%\StructuraCost\logs\`
2. Create an issue on GitHub with:
   - Windows version (run `winver`)
   - StructuraCost version
   - Full error message
   - Steps you've already tried
   - Log files

---

**Last Updated:** December 18, 2024  
**Applies to:** StructuraCost v1.0.0 and later

