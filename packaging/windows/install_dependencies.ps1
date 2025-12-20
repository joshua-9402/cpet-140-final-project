<#
Minimal installer helper for Windows packaging.
This script is packaged into the installer and executed at install time (via NSIS/CPack extra commands).
It intentionally does minimal, safe work so the CI packaging step can include it without failing.

Usage (during installer execution):
  powershell -ExecutionPolicy Bypass -File install_dependencies.ps1 -InstallDir "$INSTDIR"

Notes:
- If a companion script `install_libsodium.ps1` is present alongside this script (or in the InstallDir), this script will try to invoke it.
- This script is conservative: if the companion script is not present, it prints a message and exits 0 (successful).
- If you want the installer to *actually* download and install system-level dependencies, extend `install_libsodium.ps1` and/or add admin checks here.
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

# Look for a companion script that actually installs libsodium or other runtime deps
$scriptCandidates = @()
$scriptCandidates += Join-Path -Path $PSScriptRoot -ChildPath 'install_libsodium.ps1'
$scriptCandidates += Join-Path -Path $PSScriptRoot -ChildPath 'install_libsodium.psm1'
$scriptCandidates += Join-Path -Path $InstallDir -ChildPath 'install_libsodium.ps1'
$scriptCandidates += Join-Path -Path $InstallDir -ChildPath 'install_libsodium.psm1'

$found = $false
foreach ($c in $scriptCandidates) {
    if (Test-Path $c) {
        Write-Info "Found companion script: $c"
        try {
            & powershell -NoProfile -ExecutionPolicy Bypass -File $c -InstallDir $InstallDir
            $rc = $LASTEXITCODE
            if ($rc -ne 0) {
                Write-Warn "Companion script returned non-zero exit code: $rc"
            }
        } catch {
            Write-Warn "Failed to invoke companion script: $_"
        }
        $found = $true
        break
    }
}

if (-not $found) {
    Write-Info "No companion install_libsodium script found; skipping dependency installation."
    Write-Info "If you expect dependencies to be installed on target machines, add packaging/windows/install_libsodium.ps1 and implement the required steps."
}

# Exit successfully so packaging includes this file and installer execution doesn't abort unexpectedly.
exit 0
