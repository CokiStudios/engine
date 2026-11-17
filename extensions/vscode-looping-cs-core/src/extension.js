const vscode = require('vscode');
const path = require('path');
const fs = require('fs');
const { C2EditorProvider } = require('./c2EditorProvider');

let currentPanel = undefined;

function activate(context) {
    console.log('⚡ CS Looping Proprietary Core (C++17 WASM) & Component 2 (.c2) Viewer Active');

    const c2ProviderDisposable = C2EditorProvider.register(context);
    context.subscriptions.push(c2ProviderDisposable);

    let c2OpenDisposable = vscode.commands.registerCommand('c2.openViewer', function (uri) {
        const targetUri = uri || (vscode.window.activeTextEditor && vscode.window.activeTextEditor.document.uri);
        if (!targetUri) {
            vscode.window.showErrorMessage('No se seleccionó ningún archivo .c2');
            return;
        }
        vscode.commands.executeCommand('vscode.openWith', targetUri, 'c2.viewer');
    });

    let c2InstallDisposable = vscode.commands.registerCommand('c2.installToShine', function (uri) {
        const targetUri = uri || (vscode.window.activeTextEditor && vscode.window.activeTextEditor.document.uri);
        if (!targetUri) {
            vscode.window.showErrorMessage('No se seleccionó ningún archivo .c2');
            return;
        }
        const provider = new C2EditorProvider(context);
        provider.installToPhone(targetUri.fsPath);
    });

    let c2DeployDisposable = vscode.commands.registerCommand('c2.deployOta', function (uri) {
        const targetUri = uri || (vscode.window.activeTextEditor && vscode.window.activeTextEditor.document.uri);
        if (!targetUri) {
            vscode.window.showErrorMessage('No se seleccionó ningún archivo .c2');
            return;
        }
        const provider = new C2EditorProvider(context);
        provider.deployOta(targetUri.fsPath);
    });

    let runDisposable = vscode.commands.registerCommand('looping.runActiveFile', function () {
        const editor = vscode.window.activeTextEditor;
        if (!editor) {
            vscode.window.showErrorMessage('No active .loop file found.');
            return;
        }

        const document = editor.document;
        if (!document.fileName.endsWith('.loop') && !document.fileName.endsWith('.ruup')) {
            vscode.window.showWarningMessage('Please open a valid Looping Compile (.loop / .ruup) file.');
            return;
        }

        showLiveViewport(context, document.getText());
    });

    let buildDisposable = vscode.commands.registerCommand('looping.buildBinary', function () {
        const editor = vscode.window.activeTextEditor;
        if (!editor) {
            vscode.window.showInformationMessage('⚡ Compiling .loop -> Universal Native Binary (Windows x64 / macOS / Linux / WebAssembly)...');
            return;
        }
        const term = vscode.window.createTerminal('Looping Compiler');
        term.show();
        const exe = process.platform === 'win32' ? 'looping.exe' : 'looping';
        term.sendText(`${exe} invis "${editor.document.uri.fsPath}"`);
    });

    context.subscriptions.push(c2OpenDisposable, c2InstallDisposable, c2DeployDisposable, runDisposable, buildDisposable);
}

function resolveWasmAssetDir(context) {
    const localSrc = path.join(context.extensionPath, 'src');
    if (fs.existsSync(path.join(localSrc, 'looping_wasm.wasm'))) {
        return localSrc;
    }
    return path.resolve(context.extensionPath, '..', '..', 'web');
}

function showLiveViewport(context, initialCode) {
    const column = vscode.ViewColumn.Beside;

    if (currentPanel) {
        currentPanel.reveal(column);
        currentPanel.webview.postMessage({ command: 'updateCode', code: initialCode });
        return;
    }

    const assetDir = resolveWasmAssetDir(context);
    currentPanel = vscode.window.createWebviewPanel(
        'loopingViewport',
        'L∞ping Live Viewport (C++17 WASM) — hiOP',
        column,
        {
            enableScripts: true,
            retainContextWhenHidden: true,
            localResourceRoots: [
                vscode.Uri.file(assetDir),
                vscode.Uri.joinPath(context.extensionUri, 'src')
            ]
        }
    );

    const wv = currentPanel.webview;
    const runtimeUri = wv.asWebviewUri(vscode.Uri.file(path.join(assetDir, 'looping_runtime.js'))).toString();
    const wasmJsUri = wv.asWebviewUri(vscode.Uri.file(path.join(assetDir, 'looping_wasm.js'))).toString();
    const wasmUri = wv.asWebviewUri(vscode.Uri.file(path.join(assetDir, 'looping_wasm.wasm'))).toString();

    wv.html = getWebviewContent(wv, initialCode, { runtimeUri, wasmJsUri, wasmUri });

    currentPanel.onDidDispose(
        () => { currentPanel = undefined; },
        null,
        context.subscriptions
    );
}

function getWebviewContent(webview, code, uris) {
    return `<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta http-equiv="Content-Security-Policy" content="default-src 'none'; script-src ${webview.cspSource} 'unsafe-inline' 'wasm-unsafe-eval'; connect-src ${webview.cspSource}; style-src 'unsafe-inline'; img-src ${webview.cspSource} data:;">
    <title>Looping C++17 WASM Viewport</title>
    <style>
        body { margin: 0; background: #06090f; color: #f1f5f9; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif; display: flex; flex-direction: column; height: 100vh; overflow: hidden; }
        .header { height: 40px; background: #0d1117; display: flex; align-items: center; justify-content: space-between; padding: 0 14px; border-bottom: 1px solid rgba(255,255,255,0.1); font-size: 12px; font-weight: bold; }
        .badge { background: linear-gradient(135deg, #38bdf8, #8b5cf6); color: #020617; padding: 3px 9px; border-radius: 99px; font-size: 10px; font-weight: 800; }
        .controls { display: flex; gap: 8px; align-items: center; }
        .btn { background: rgba(56,189,248,0.15); border: 1px solid rgba(56,189,248,0.4); color: #e2e8f0; border-radius: 6px; padding: 3px 9px; font-size: 11px; cursor: pointer; font-weight: 600; }
        .btn:hover { background: rgba(56,189,248,0.3); }
        .canvas-container { flex: 1; display: flex; align-items: center; justify-content: center; background: #04060a; padding: 10px; }
        canvas { max-width: 100%; max-height: 100%; background: #000; border-radius: 8px; box-shadow: 0 10px 30px rgba(0,0,0,0.6); }
        .terminal { height: 135px; background: #080c14; border-top: 1px solid rgba(255,255,255,0.1); padding: 8px 14px; font-family: ui-monospace, monospace; font-size: 11.5px; overflow-y: auto; color: #38bdf8; white-space: pre-wrap; }
    </style>
</head>
<body>
    <div class="header">
        <span>L∞ping C++17 WebAssembly Viewport</span>
        <div class="controls">
            <button class="btn" id="btn-restart">↻ Restart</button>
            <button class="btn" id="btn-cam">📷 Cam [V]</button>
            <span class="badge">C++17 WASM CORE</span>
        </div>
    </div>
    <div class="canvas-container">
        <canvas id="gameCanvas" width="760" height="440"></canvas>
    </div>
    <div class="terminal" id="termLogs">> Loading Looping C++17 WebAssembly Engine...</div>

    <script type="module">
        import { LoopingWasmRuntime } from ${JSON.stringify(uris.runtimeUri)};
        const canvas = document.getElementById('gameCanvas');
        const term = document.getElementById('termLogs');

        function log(msg) {
            const line = document.createElement('div');
            line.textContent = '> ' + msg;
            term.appendChild(line);
            term.scrollTop = term.scrollHeight;
        }

        const runtime = new LoopingWasmRuntime(canvas, {
            onLog: log,
            wasmJsUrl: ${JSON.stringify(uris.wasmJsUri)},
            wasmUrl: ${JSON.stringify(uris.wasmUri)}
        });

        await runtime.ready;
        const ver = runtime.getVersion();
        log(\`Engine ready: \${ver.engine} v\${ver.version} (Arch \${ver.arch}) — \${ver.runtime}\`);
        runtime.execute(${JSON.stringify(code)});

        document.getElementById('btn-restart').addEventListener('click', () => runtime.restartGame());
        document.getElementById('btn-cam').addEventListener('click', () => {
            if (runtime.is3DMode) runtime.cycleCamera3D();
            else runtime.toggle3DLayer();
        });

        window.addEventListener('message', event => {
            if (event.data && event.data.command === 'updateCode') {
                term.innerHTML = '';
                runtime.execute(event.data.code || '');
            }
        });
    </script>
</body>
</html>`;
}

function deactivate() {}

module.exports = {
    activate,
    deactivate
};
