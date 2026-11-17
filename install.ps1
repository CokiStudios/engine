#Requires -Version 5.1
<#
.SYNOPSIS
    Looping Engine & smiledev CLI — Official Windows Installer (v1.1.2 / Arch 3.1A)
.DESCRIPTION
    Installs Looping Engine CLI (looping.exe, ruuping.cmd) and smiledev package manager
    (smiledev.cmd, smiledev.ps1, smiledev Bash script) into %USERPROFILE%\.smiledev\bin
    and adds it cleanly to the User PATH. Builds from C++17 source if requested or if
    no prebuilt binary is present, or fetches the official Windows x64 archive when run
    standalone via repo.cokistudios.com/engine.
#>
[CmdletBinding()]
param(
    [string]$Prefix = (Join-Path $env:USERPROFILE ".smiledev"),
    [switch]$FromSource,
    [switch]$NoPath,
    [switch]$Verify
)

$ErrorActionPreference = "Stop"

$RepoRoot = if ($PSScriptRoot) { $PSScriptRoot } else { (Get-Location).Path }
$BinSrc = Join-Path $RepoRoot "bin"
$BinDest = Join-Path $Prefix "bin"

Write-Host ""
Write-Host "  ================================================================" -ForegroundColor Cyan
Write-Host "   Looping Engine v1.1.2 (Arch 3.1A) & smiledev -- Windows Installer" -ForegroundColor Cyan
Write-Host "  ================================================================" -ForegroundColor Cyan
Write-Host ""

# Create workspace directories
$Dirs = @(
    $Prefix,
    $BinDest,
    (Join-Path $Prefix "config"),
    (Join-Path $Prefix "cache"),
    (Join-Path $Prefix "modules"),
    (Join-Path $Prefix "projects"),
    (Join-Path $Prefix "templates")
)
foreach ($d in $Dirs) {
    if (-not (Test-Path $d)) {
        New-Item -ItemType Directory -Path $d -Force | Out-Null
    }
}

$ExeSrc = Join-Path $BinSrc "looping.exe"
$DownloadedTempDir = $null
if ($FromSource -or (-not (Test-Path $ExeSrc))) {
    $BuildScript = Join-Path $RepoRoot "scripts\build-windows.ps1"
    if (Test-Path $BuildScript) {
        Write-Host "[build] Compiling Looping C++17 Engine from source..." -ForegroundColor Yellow
        & $BuildScript
    } elseif (-not $FromSource) {
        $ZipUrl = "https://repo.cokistudios.com/engine/looping-v1.1.2-windows-x64.zip"
        $FallbackZipUrl = "https://cokistudios.github.io/engine/looping-v1.1.2-windows-x64.zip"
        $DownloadedTempDir = Join-Path ([System.IO.Path]::GetTempPath()) ("looping-win-install-" + [Guid]::NewGuid().ToString("N"))
        New-Item -ItemType Directory -Path $DownloadedTempDir -Force | Out-Null
        $ZipFile = Join-Path $DownloadedTempDir "looping-windows-x64.zip"
        Write-Host "[download] Fetching Looping v1.1.2 (Arch 3.1A) for Windows x64 from repo.cokistudios.com/engine..." -ForegroundColor Yellow
        try {
            Invoke-WebRequest -Uri $ZipUrl -OutFile $ZipFile -UseBasicParsing
        } catch {
            Invoke-WebRequest -Uri $FallbackZipUrl -OutFile $ZipFile -UseBasicParsing
        }
        Expand-Archive -Path $ZipFile -DestinationPath $DownloadedTempDir -Force
        $BinSrc = $DownloadedTempDir
    } else {
        Write-Warning "scripts\build-windows.ps1 not found and looping.exe missing."
    }
}

# Copy binaries & wrappers
$FilesToCopy = @(
    "looping.exe",
    "ruuping.cmd",
    "smiledev",
    "smiledev.cmd",
    "smiledev.ps1"
)
foreach ($f in $FilesToCopy) {
    $srcPath = Join-Path $BinSrc $f
    if (Test-Path $srcPath) {
        Copy-Item -Path $srcPath -Destination (Join-Path $BinDest $f) -Force
        Write-Host "[ok] Installed $f -> $BinDest" -ForegroundColor Green
    } else {
        Write-Host "[skip] Optional file not present: $f" -ForegroundColor DarkGray
    }
}
if ($DownloadedTempDir -and (Test-Path $DownloadedTempDir)) {
    Remove-Item -Path $DownloadedTempDir -Recurse -Force -ErrorAction SilentlyContinue
}

# Install default bundled modules if present
$ModulesSrc = Join-Path $RepoRoot "modules"
if (Test-Path $ModulesSrc) {
    $ModDest = Join-Path $Prefix "modules"
    Copy-Item -Path (Join-Path $ModulesSrc "*") -Destination $ModDest -Recurse -Force
    Write-Host "[ok] Bundled modules synced to $ModDest" -ForegroundColor Green
}

# Add %USERPROFILE%\.smiledev\bin to User PATH
if (-not $NoPath) {
    $CurrentUserPath = [Environment]::GetEnvironmentVariable("Path", "User")
    $Parts = @()
    if ($CurrentUserPath) {
        $Parts = $CurrentUserPath.Split(";", [System.StringSplitOptions]::RemoveEmptyEntries)
    }
    $AlreadyInPath = $Parts | Where-Object { $_.TrimEnd("\") -ieq $BinDest.TrimEnd("\") }
    if (-not $AlreadyInPath) {
        $NewPath = if ($CurrentUserPath) { "$CurrentUserPath;$BinDest" } else { $BinDest }
        [Environment]::SetEnvironmentVariable("Path", $NewPath, "User")
        $env:PATH = "$env:PATH;$BinDest"
        Write-Host "[ok] Added $BinDest to User PATH" -ForegroundColor Green
    } else {
        Write-Host "[ok] $BinDest is already in User PATH" -ForegroundColor Green
    }
}

if ($Verify) {
    $InstalledLooping = Join-Path $BinDest "looping.exe"
    if (Test-Path $InstalledLooping) {
        Write-Host "[verify] Running looping.exe --version & test..." -ForegroundColor Cyan
        & $InstalledLooping --version
        & $InstalledLooping test
    }
    $InstalledSmile = Join-Path $BinDest "smiledev.ps1"
    if (Test-Path $InstalledSmile) {
        & $InstalledSmile version
    }
}

Write-Host ""
Write-Host "Installation complete! Open a new PowerShell/Terminal and run:" -ForegroundColor Cyan
Write-Host "  looping --version"
Write-Host "  looping doctor"
Write-Host "  smiledev help"
Write-Host ""
