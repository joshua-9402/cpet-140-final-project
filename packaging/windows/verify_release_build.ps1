# Verify that the executable is not linking against debug runtime DLLs
param(
    [Parameter(Mandatory=$true)]
    [string]$ExecutablePath
)

Write-Host "Checking executable dependencies for: $ExecutablePath" -ForegroundColor Cyan
Write-Host ""

if (-not (Test-Path $ExecutablePath)) {
    Write-Host "ERROR: Executable not found: $ExecutablePath" -ForegroundColor Red
    exit 1
}

# Check if dumpbin is available
$dumpbin = Get-Command dumpbin -ErrorAction SilentlyContinue

if ($dumpbin) {
    Write-Host "Using dumpbin to check dependencies..." -ForegroundColor Yellow
    Write-Host ""

    # Get dependencies
    $deps = & dumpbin /dependents "$ExecutablePath" 2>&1

    # Display all DLL dependencies
    $deps | Select-String "\.dll" | ForEach-Object {
        Write-Host "  $_" -ForegroundColor Gray
    }
    Write-Host ""

    # Check for debug runtime DLLs
    $hasDebugRuntime = $false
    if ($deps -match "ucrtbased\.dll|vcruntime140d\.dll|msvcp140d\.dll") {
        Write-Host "❌ ERROR: Executable is linked against DEBUG runtime libraries!" -ForegroundColor Red
        Write-Host ""

        if ($deps -match "ucrtbased\.dll") {
            Write-Host "  Found: ucrtbased.dll (Debug Universal C Runtime)" -ForegroundColor Red
        }
        if ($deps -match "vcruntime140d\.dll") {
            Write-Host "  Found: vcruntime140d.dll (Debug VC++ Runtime)" -ForegroundColor Red
        }
        if ($deps -match "msvcp140d\.dll") {
            Write-Host "  Found: msvcp140d.dll (Debug C++ Standard Library)" -ForegroundColor Red
        }

        Write-Host ""
        Write-Host "This is a RELEASE build but it's using DEBUG runtime libraries." -ForegroundColor Red
        Write-Host "Please check CMake configuration and ensure:" -ForegroundColor Yellow
        Write-Host "  1. CMAKE_BUILD_TYPE=Release" -ForegroundColor Yellow
        Write-Host "  2. CMAKE_MSVC_RUNTIME_LIBRARY is set to MultiThreadedDLL" -ForegroundColor Yellow
        Write-Host "  3. All dependencies are also built in Release mode" -ForegroundColor Yellow
        Write-Host ""

        $hasDebugRuntime = $true
    }

    # Check for release runtime DLLs
    $hasReleaseRuntime = $false
    if ($deps -match "ucrtbase\.dll|vcruntime140\.dll") {
        if (-not $hasDebugRuntime) {
            Write-Host "✅ SUCCESS: Using correct RELEASE runtime libraries" -ForegroundColor Green
            Write-Host ""
        }

        if ($deps -match "ucrtbase\.dll") {
            Write-Host "  ✓ ucrtbase.dll (Release Universal C Runtime)" -ForegroundColor Green
        }
        if ($deps -match "vcruntime140\.dll") {
            Write-Host "  ✓ vcruntime140.dll (Release VC++ Runtime)" -ForegroundColor Green
        }
        if ($deps -match "msvcp140\.dll") {
            Write-Host "  ✓ msvcp140.dll (Release C++ Standard Library)" -ForegroundColor Green
        }

        $hasReleaseRuntime = $true
    }

    Write-Host ""

    if ($hasDebugRuntime) {
        exit 1
    } elseif ($hasReleaseRuntime) {
        Write-Host "Build verification PASSED ✓" -ForegroundColor Green
        exit 0
    } else {
        Write-Host "Warning: Could not determine runtime library type" -ForegroundColor Yellow
        exit 0
    }

} else {
    Write-Host "Warning: dumpbin not found. Skipping dependency verification." -ForegroundColor Yellow
    Write-Host "(dumpbin is part of Visual Studio Build Tools)" -ForegroundColor Gray
    Write-Host ""
    exit 0
}

