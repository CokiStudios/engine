#Requires -Version 5.1
<#
.SYNOPSIS
    Build Looping C++17 WebAssembly Engine (web/looping_wasm.{js,wasm}) via native em++ or WSL emsdk
#>
[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$RepoRoot = Split-Path -Parent $PSScriptRoot

$Flags = @(
    "-std=c++17", "-O3", "-sDISABLE_EXCEPTION_CATCHING=0", "-Icpp", "cpp/LoopingWasm.cpp",
    "-o", "web/looping_wasm.js",
    "-sWASM=1", "-sMODULARIZE=1", "-sEXPORT_ES6=1", "-sEXPORT_NAME=createLoopingWasm",
    "-sALLOW_MEMORY_GROWTH=1", "-sNO_EXIT_RUNTIME=1", "-sENVIRONMENT=web,worker,node",
    "-sEXPORTED_RUNTIME_METHODS=ccall,cwrap,UTF8ToString,HEAPU8"
)

if (Get-Command em++ -ErrorAction SilentlyContinue) {
    Push-Location $RepoRoot
    try {
        Write-Host "[wasm] Compiling with native em++..." -ForegroundColor Cyan
        & em++ @Flags
        if ($LASTEXITCODE -ne 0) { throw "em++ failed with exit code $LASTEXITCODE" }
    } finally {
        Pop-Location
    }
}
elseif (Get-Command wsl -ErrorAction SilentlyContinue) {
    $unixFwd = $RepoRoot -replace '\\', '/'
    $wslPath = [regex]::Replace($unixFwd, '^([A-Za-z]):', { param($m) "/mnt/" + $m.Groups[1].Value.ToLower() })
    $joinedFlags = $Flags -join " "
    Write-Host "[wasm] Compiling via WSL emsdk ($wslPath)..." -ForegroundColor Cyan
    & wsl -d Ubuntu -- bash -c "source ~/emsdk/emsdk_env.sh >/dev/null 2>&1 && cd '$wslPath' && em++ $joinedFlags"
    if ($LASTEXITCODE -ne 0) { throw "WSL em++ failed with exit code $LASTEXITCODE" }
}
else {
    throw "Neither native em++ nor WSL Ubuntu emsdk was found."
}

$ExtSrc = Join-Path $RepoRoot "extensions\vscode-looping-cs-core\src"
if (Test-Path $ExtSrc) {
    Copy-Item (Join-Path $RepoRoot "web\looping_wasm.js") $ExtSrc -Force
    Copy-Item (Join-Path $RepoRoot "web\looping_wasm.wasm") $ExtSrc -Force
    Copy-Item (Join-Path $RepoRoot "web\looping_runtime.js") $ExtSrc -Force
    Write-Host "[ok] Synced WASM assets -> $ExtSrc" -ForegroundColor Green
}

Write-Host "[ok] WASM build complete!" -ForegroundColor Green
