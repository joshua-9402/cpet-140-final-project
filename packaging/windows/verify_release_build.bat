@echo off
REM Verify that the executable is not linking against debug runtime DLLs

echo Checking executable dependencies...

REM Use dumpbin if available (comes with Visual Studio)
where dumpbin >nul 2>&1
if %ERRORLEVEL% EQU 0 (
    echo.
    echo Using dumpbin to check dependencies:
    dumpbin /dependents "%~1" | findstr /I "dll"
    echo.

    REM Check for debug DLLs
    dumpbin /dependents "%~1" | findstr /I "ucrtbased.dll vcruntime140d.dll" >nul 2>&1
    if %ERRORLEVEL% EQU 0 (
        echo ERROR: Executable is linked against DEBUG runtime libraries!
        echo ERROR: Found ucrtbased.dll or vcruntime140d.dll
        echo.
        echo This is a RELEASE build but it's using DEBUG runtime libraries.
        echo Please check CMake configuration.
        exit /b 1
    ) else (
        echo SUCCESS: No debug runtime DLLs detected

        REM Verify release DLLs are present
        dumpbin /dependents "%~1" | findstr /I "ucrtbase.dll vcruntime140.dll" >nul 2>&1
        if %ERRORLEVEL% EQU 0 (
            echo SUCCESS: Using correct RELEASE runtime libraries (ucrtbase.dll, vcruntime140.dll)
        )
    )
) else (
    echo Warning: dumpbin not found. Skipping dependency verification.
    echo (dumpbin is part of Visual Studio Build Tools)
)

exit /b 0

