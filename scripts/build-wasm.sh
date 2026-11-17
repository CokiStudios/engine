#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

if ! command -v em++ >/dev/null 2>&1; then
    for candidate in "$HOME/emsdk/emsdk_env.sh" "/opt/emsdk/emsdk_env.sh"; do
        if [ -f "$candidate" ]; then
            # shellcheck disable=SC1090
            source "$candidate" >/dev/null 2>&1
            break
        fi
    done
fi

if ! command -v em++ >/dev/null 2>&1; then
    echo "[error] em++ (Emscripten SDK) not found in PATH." >&2
    exit 1
fi

echo "[wasm] Compiling C++17 Looping Engine -> web/looping_wasm.{js,wasm}..."
em++ -std=c++17 -O3 -sDISABLE_EXCEPTION_CATCHING=0 -Icpp cpp/LoopingWasm.cpp \
    -o web/looping_wasm.js \
    -sWASM=1 \
    -sMODULARIZE=1 \
    -sEXPORT_ES6=1 \
    -sEXPORT_NAME=createLoopingWasm \
    -sALLOW_MEMORY_GROWTH=1 \
    -sNO_EXIT_RUNTIME=1 \
    -sENVIRONMENT=web,worker,node \
    -sEXPORTED_RUNTIME_METHODS=ccall,cwrap,UTF8ToString,HEAPU8

echo "[wasm] Copying WASM runtime assets to VS Code extension & loop-node-window..."
mkdir -p extensions/vscode-looping-cs-core/src
cp -f web/looping_wasm.js web/looping_wasm.wasm web/looping_runtime.js extensions/vscode-looping-cs-core/src/

echo "[ok] Built web/looping_wasm.js & web/looping_wasm.wasm"
ls -lh web/looping_wasm.js web/looping_wasm.wasm web/looping_runtime.js
