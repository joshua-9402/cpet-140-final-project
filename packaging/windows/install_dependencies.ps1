<#
Minimal installer helper for Windows packaging.
This script is packaged into the installer and executed at install time (via NSIS/CPack extra commands).
It intentionally does minimal, safe work so the CI packaging step can include it without failing.

Usage (during installer execution):
  powershell -ExecutionPolicy Bypass -File install_dependencies.ps1 -InstallDir "$INSTDIR"

Notes:
- This script intentionally does NOT attempt to install libsodium or other optional runtime dependencies.
- If you want the installer to install additional runtime dependencies, add companion scripts under packaging/windows and invoke them from your installer packaging logic outside of this file.
#>
param(
    [string]$InstallDir = ".",
    [switch]$Quiet
)

function Write-Info {
    param(
        [Parameter(ValueFromRemainingArguments=$true)]
        [object[]]$Message
    )
    if (-not $Quiet) {
        Write-Host ($Message -join ' ')
    }
}
function Write-Warn {
    param(
        [Parameter(ValueFromRemainingArguments=$true)]
        [object[]]$Message
    )
    Write-Warning ($Message -join ' ')
}
function Write-Err {
    param(
        [Parameter(ValueFromRemainingArguments=$true)]
        [object[]]$Message
    )
    Write-Error ($Message -join ' ')
}

Write-Info "install_dependencies.ps1: invoked. InstallDir='$InstallDir'"

# This packaging helper intentionally does not auto-install third-party runtime libraries.
# If your installer needs to bundle additional DLLs, include them in the installer payload and copy them here.

# Ensure destination exists
try {
    if (-not (Test-Path -Path $InstallDir)) {
        New-Item -ItemType Directory -Force -Path $InstallDir | Out-Null
        Write-Info "Created install directory: $InstallDir"
    }
} catch {
    Write-Err ("Failed to ensure InstallDir exists: {0}" -f $_.Exception.Message)
    exit 2
}

# Exit successfully so packaging includes this file and installer execution doesn't abort unexpectedly.
exit 0
