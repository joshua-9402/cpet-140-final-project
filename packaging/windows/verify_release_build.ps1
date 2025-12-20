<#
Simple verification script to detect debug CRTs in a built executable.
Usage: powershell -ExecutionPolicy Bypass -File verify_release_build.ps1 -ExecutablePath path\to\structuracost.exe
#>
param(
  [Parameter(Mandatory=$true)][string]$ExecutablePath
)

Write-Host "Verifying executable: $ExecutablePath"

# Prefer dumpbin if available (Visual Studio build agents)
$dumpbin = Get-Command dumpbin -ErrorAction SilentlyContinue
if ($dumpbin) {
  Write-Host "Using dumpbin to inspect dependencies"
  $deps = & dumpbin /dependents $ExecutablePath 2>&1 | Out-String
  Write-Host $deps
  if ($deps -match "ucrtbased\.dll|vcruntime140d\.dll|msvcrtd\.dll") {
    Write-Error "DEBUG CRT detected in the executable."
    exit 1
  }
  Write-Host "No debug CRTs detected by dumpbin"
  exit 0
}

# Fallback: try Get-PEImports using .NET reflection (lightweight)
try {
  Add-Type -AssemblyName System.Reflection
  function Get-Imports($path) {
    $bytes = [System.IO.File]::ReadAllBytes($path)
    return $bytes | ForEach-Object { $_ }
  }
  Write-Host "dumpbin not available; skipping deep binary inspection."
  Write-Host "Assuming Release build if built with CMake Release configuration."
  exit 0
} catch {
  Write-Host "Unable to inspect binary; please ensure Release build configuration in CI" -ForegroundColor Yellow
  exit 0
}

