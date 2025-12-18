# StructuraCost - Windows Dependencies Auto-Installer
# PowerShell script to install all required dependencies

param(
    [string]$InstallDir = "$env:ProgramFiles\StructuraCost"
)

$ErrorActionPreference = "Stop"
$ProgressPreference = 'SilentlyContinue'

Write-Host ""
Write-Host "================================================" -ForegroundColor Cyan
Write-Host "  StructuraCost - Installing Dependencies" -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan
Write-Host ""

# Function to check if VC++ Redistributables are installed
function Test-VCRedist {
    param([string]$Version)

    $registryPaths = @(
        "HKLM:\SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64",
        "HKLM:\SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x86",
        "HKLM:\SOFTWARE\WOW6432Node\Microsoft\VisualStudio\14.0\VC\Runtimes\x64",
        "HKLM:\SOFTWARE\WOW6432Node\Microsoft\VisualStudio\14.0\VC\Runtimes\x86"
    )

    foreach ($path in $registryPaths) {
        if (Test-Path $path) {
            $installed = Get-ItemProperty -Path $path -Name Installed -ErrorAction SilentlyContinue
            if ($installed.Installed -eq 1) {
                return $true
            }
        }
    }
    return $false
}

# Function to download file with progress
function Download-File {
    param(
        [string]$Url,
        [string]$OutputPath
    )

    try {
        Write-Host "  Downloading: $(Split-Path $OutputPath -Leaf)" -ForegroundColor Yellow
        $webClient = New-Object System.Net.WebClient
        $webClient.DownloadFile($Url, $OutputPath)
        Write-Host "  ✓ Downloaded successfully" -ForegroundColor Green
        return $true
    } catch {
        Write-Host "  ✗ Download failed: $_" -ForegroundColor Red
        return $false
    }
}

# Function to install VC++ Redistributable
function Install-VCRedist {
    param(
        [string]$Architecture,
        [string]$Url
    )

    Write-Host ""
    Write-Host "Installing Visual C++ Redistributable ($Architecture)..." -ForegroundColor Cyan

    if (Test-VCRedist -Version "2015-2022") {
        Write-Host "  ✓ Already installed" -ForegroundColor Green
        return $true
    }

    $tempDir = Join-Path $env:TEMP "vcredist_$(Get-Random)"
    $installerPath = Join-Path $tempDir "vc_redist.$Architecture.exe"

    try {
        New-Item -ItemType Directory -Path $tempDir -Force | Out-Null

        if (Download-File -Url $Url -OutputPath $installerPath) {
            Write-Host "  Installing (this may take a few minutes)..." -ForegroundColor Yellow
            $process = Start-Process -FilePath $installerPath -ArgumentList "/install", "/quiet", "/norestart" -Wait -PassThru

            if ($process.ExitCode -eq 0 -or $process.ExitCode -eq 3010) {
                Write-Host "  ✓ Installed successfully" -ForegroundColor Green
                return $true
            } elseif ($process.ExitCode -eq 1638) {
                Write-Host "  ✓ Already installed (newer version)" -ForegroundColor Green
                return $true
            } else {
                Write-Host "  ✗ Installation failed (Exit code: $($process.ExitCode))" -ForegroundColor Red
                return $false
            }
        }
        return $false
    } catch {
        Write-Host "  ✗ Error: $_" -ForegroundColor Red
        return $false
    } finally {
        if (Test-Path $tempDir) {
            Remove-Item -Path $tempDir -Recurse -Force -ErrorAction SilentlyContinue
        }
    }
}

# Function to install libsodium
function Install-Libsodium {
    Write-Host ""
    Write-Host "Installing libsodium..." -ForegroundColor Cyan

    if (Test-Path "$InstallDir\libsodium.dll") {
        Write-Host "  ✓ Already installed" -ForegroundColor Green
        return $true
    }

    $arch = if ([Environment]::Is64BitOperatingSystem) { "x64" } else { "x86" }
    $version = "1.0.20"
    $url = "https://download.libsodium.org/libsodium/releases/libsodium-$version-stable-msvc.zip"
    $tempDir = Join-Path $env:TEMP "libsodium_install_$(Get-Random)"
    $zipFile = Join-Path $tempDir "libsodium.zip"

    try {
        New-Item -ItemType Directory -Path $tempDir -Force | Out-Null

        if (Download-File -Url $url -OutputPath $zipFile) {
            Write-Host "  Extracting..." -ForegroundColor Yellow
            Expand-Archive -Path $zipFile -DestinationPath $tempDir -Force

            $dllPath = if ($arch -eq "x64") {
                Get-ChildItem -Path $tempDir -Filter "libsodium.dll" -Recurse |
                    Where-Object { $_.FullName -like "*x64*Release*dynamic*" } |
                    Select-Object -First 1
            } else {
                Get-ChildItem -Path $tempDir -Filter "libsodium.dll" -Recurse |
                    Where-Object { $_.FullName -like "*Win32*Release*dynamic*" } |
                    Select-Object -First 1
            }

            if (-not $dllPath) {
                $dllPath = Get-ChildItem -Path $tempDir -Filter "libsodium.dll" -Recurse |
                    Select-Object -First 1
            }

            if ($dllPath) {
                Copy-Item -Path $dllPath.FullName -Destination "$InstallDir\libsodium.dll" -Force
                Write-Host "  ✓ Installed successfully" -ForegroundColor Green
                return $true
            } else {
                Write-Host "  ✗ Could not find libsodium.dll in archive" -ForegroundColor Red
                return $false
            }
        }
        return $false
    } catch {
        Write-Host "  ✗ Error: $_" -ForegroundColor Red
        return $false
    } finally {
        if (Test-Path $tempDir) {
            Remove-Item -Path $tempDir -Recurse -Force -ErrorAction SilentlyContinue
        }
    }
}

# Main installation process
$allSuccess = $true

# Install Visual C++ Redistributables
$vcx64Success = Install-VCRedist -Architecture "x64" -Url "https://aka.ms/vs/17/release/vc_redist.x64.exe"
$vcx86Success = Install-VCRedist -Architecture "x86" -Url "https://aka.ms/vs/17/release/vc_redist.x86.exe"

if (-not $vcx64Success -or -not $vcx86Success) {
    $allSuccess = $false
}

# Install libsodium
$sodiumSuccess = Install-Libsodium
if (-not $sodiumSuccess) {
    $allSuccess = $false
}

# Summary
Write-Host ""
Write-Host "================================================" -ForegroundColor Cyan
Write-Host "  Installation Summary" -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan
Write-Host ""

if ($allSuccess) {
    Write-Host "✓ All dependencies installed successfully!" -ForegroundColor Green
    Write-Host ""
    Write-Host "StructuraCost is ready to use." -ForegroundColor Green
} else {
    Write-Host "⚠ Some dependencies could not be installed automatically" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Please install manually:" -ForegroundColor Yellow

    if (-not $vcx64Success -or -not $vcx86Success) {
        Write-Host "  • Visual C++ Redistributable:" -ForegroundColor Yellow
        Write-Host "    https://aka.ms/vs/17/release/vc_redist.x64.exe" -ForegroundColor White
        Write-Host "    https://aka.ms/vs/17/release/vc_redist.x86.exe" -ForegroundColor White
    }

    if (-not $sodiumSuccess) {
        Write-Host "  • libsodium:" -ForegroundColor Yellow
        Write-Host "    See README_WINDOWS.txt in installation folder" -ForegroundColor White
    }

    Write-Host ""
    Write-Host "For detailed instructions, see:" -ForegroundColor Yellow
    Write-Host "  $InstallDir\README_WINDOWS.txt" -ForegroundColor White
}

Write-Host ""
Write-Host "Press any key to continue..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")

