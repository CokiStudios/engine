#Requires -Version 5.1
<#
.SYNOPSIS
    Build Looping Engine C++17 Native Windows Binary (bin\looping.exe)
#>
[CmdletBinding()]
param(
    [ValidateSet("auto", "msvc", "mingw", "clang", "cmake")]
    [string]$Toolchain = "auto",
    [switch]$RunTests
)

$ErrorActionPreference = "Stop"
$RepoRoot = Split-Path -Parent $PSScriptRoot
$CppDir = Join-Path $RepoRoot "cpp"
$BinDir = Join-Path $RepoRoot "bin"
$BuildDir = Join-Path $RepoRoot "build\win"
$OutExe = Join-Path $BinDir "looping.exe"

New-Item -ItemType Directory -Force -Path $BinDir | Out-Null
New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null

function Find-VsVars64 {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $inst = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2>$null
        if ($inst) {
            $vc = Join-Path $inst "VC\Auxiliary\Build\vcvars64.bat"
            if (Test-Path $vc) { return $vc }
        }
    }
    $fallback = @(
        "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat",
        "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat",
        "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat",
        "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat",
        "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
    )
    foreach ($p in $fallback) {
        if (Test-Path $p) { return $p }
    }
    return $null
}

$ObjFile = Join-Path $BuildDir "main.obj"

if (($Toolchain -in @("msvc", "auto")) -and (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    Write-Host "[build-windows] Building with active MSVC cl.exe..." -ForegroundColor Cyan
    & cl.exe /nologo /std:c++17 /EHsc /utf-8 /O2 "/I$CppDir" "/Fo:$ObjFile" "$CppDir\main.cpp" /link "/OUT:$OutExe"
    if ($LASTEXITCODE -ne 0) { throw "MSVC cl.exe failed with exit code $LASTEXITCODE" }
}
elseif ($Toolchain -eq "msvc" -or $Toolchain -eq "auto") {
    $vcvars = Find-VsVars64
    if ($vcvars) {
        Write-Host "[build-windows] Initializing MSVC via $vcvars..." -ForegroundColor Cyan
        $cmdLine = "`"$vcvars`" && cl.exe /nologo /std:c++17 /EHsc /utf-8 /O2 /I`"$CppDir`" /Fo:`"$ObjFile`" `"$CppDir\main.cpp`" /link /OUT:`"$OutExe`""
        cmd.exe /c $cmdLine
        if ($LASTEXITCODE -ne 0) { throw "MSVC build failed with exit code $LASTEXITCODE" }
    }
    elseif (Get-Command g++ -ErrorAction SilentlyContinue) {
        $Toolchain = "mingw"
    }
    elseif (Get-Command clang++ -ErrorAction SilentlyContinue) {
        $Toolchain = "clang"
    }
    else {
        throw "No C++17 compiler found (MSVC cl.exe, MinGW g++, or clang++)."
    }
}

if ($Toolchain -eq "mingw") {
    Write-Host "[build-windows] Building with MinGW g++..." -ForegroundColor Cyan
    & g++ -std=c++17 -O2 -I"$CppDir" "$CppDir\main.cpp" -o "$OutExe"
    if ($LASTEXITCODE -ne 0) { throw "g++ failed with exit code $LASTEXITCODE" }
}
elseif ($Toolchain -eq "clang") {
    Write-Host "[build-windows] Building with clang++..." -ForegroundColor Cyan
    & clang++ -std=c++17 -O2 -I"$CppDir" "$CppDir\main.cpp" -o "$OutExe"
    if ($LASTEXITCODE -ne 0) { throw "clang++ failed with exit code $LASTEXITCODE" }
}
elseif ($Toolchain -eq "cmake") {
    Write-Host "[build-windows] Building with CMake..." -ForegroundColor Cyan
    & cmake -S "$CppDir" -B "$BuildDir\cmake"
    if ($LASTEXITCODE -ne 0) { throw "cmake configure failed" }
    & cmake --build "$BuildDir\cmake" --config Release
    if ($LASTEXITCODE -ne 0) { throw "cmake build failed" }
}

Write-Host "[ok] Built $OutExe ($([math]::Round((Get-Item $OutExe).Length / 1KB, 1)) KB)" -ForegroundColor Green

if ($RunTests) {
    & $OutExe --version
    & $OutExe test
    if ($LASTEXITCODE -ne 0) { throw "looping.exe test failed" }
}
