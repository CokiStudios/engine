# ⚡ Looping Engine & smiledev
> **Looping v1.1 — Production Release & Alpha! 1.1A for smiledev (Architecture 3.0A Quantum 3D Synthesis)**  
> *The Dual Hybrid High-Performance 2D/3D Language & Game Engine by Coki Studios & Holo Entertainment*

[![Release](https://img.shields.io/badge/release-v1.1%20Production-38bdf8.svg?style=flat-square)](https://github.com/CokiStudios/engine/releases/tag/v1.1)
[![Alpha](https://img.shields.io/badge/alpha!-1.1A%20smiledev-f43f5e.svg?style=flat-square)](https://repo.cokistudios.com/engine)
[![Architecture](https://img.shields.io/badge/arch-3.0A%20Quantum%203D-10b981.svg?style=flat-square)](https://cokistudios.github.io/developer.html)
[![Tests](https://img.shields.io/badge/tests-71%2F71%20passing-a855f7.svg?style=flat-square)](https://github.com/CokiStudios/engine)
[![License](https://img.shields.io/badge/license-MIT-f59e0b.svg?style=flat-square)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-macOS%20Universal%20%7C%20Linux%20%7C%20POSIX-64748b.svg?style=flat-square)](https://github.com/CokiStudios/engine)

---

## 🏷️ Nomenclatura Oficial de Canales (Looping Release Tiers)

El ecosistema Looping se distribuye bajo **tres niveles oficiales de nomenclatura**, accesibles mediante `--channel=<prod|omega|alpha>` en `looping` y `smiledev`:

| Canal / Nombre Oficial | Rol | Flag CLI | Descripción |
| :--- | :--- | :--- | :--- |
| ⚡ **`Looping`** | **Producción (Stable)** | `--channel=prod` | Versión oficial estable de producción (**actual: `v1.1` / `Arch 3.0A`**). Lista para despliegue comercial y aplicaciones críticas. |
| 🧬 **`Omega Looping`** | **Release Candidate (RC)** | `--channel=omega` | Builds candidatas a lanzamiento (RC) con congelación de APIs y validación QA previa a promoción a producción (`v1.1-omega`). |
| 🧪 **`Alpha! 1.1A`** | **Super Experimental** | `--channel=alpha` | Builds súper experimentales de laboratorio para `smiledev` (`Alpha! 1.1A`) y `Alpha! Looping` (`v1.1-alpha`) con Metal 4, terminal persistente (`-i`) y prototipado Arch 3.0A. |

---

## 🌟 Visión General

**Looping** (`.loop` / `.ruup`) es un lenguaje de programación moderno, tipado dinámico y de altísimo rendimiento diseñado para la creación ágil de:
* **Motor Cuántico de Videojuegos 3D (`Looping3D.hpp` — Arch 3.0A)**: Álgebra lineal 3D (`math3d.*`), cuaterniones (`quat_euler`, `quat_slerp`), matrices `Mat4` TRS, colisiones volumétricas (Esfera, AABB, Cápsula), Raycasting 3D (`physics3d.raycast_scene`), física de cuerpos rígidos con sub-stepping a **120Hz/240Hz**, iluminación Blinn-Phong y rasterizador software Z-Buffer TrueColor en terminal (`looping preview3d`) y Canvas Web.
* **Videojuegos 2D con Física Cuántica**: Motor a 60/120 FPS con coyote time, jump buffering, dash con estelas, pisoteo de enemigos, plataformas móviles maglev, sintetizador procedural Web Audio y controles táctiles/gamepad.
* **Aplicaciones Nativas, Snippets Reactivos & SwiftUI**: Plantillas dinámicas (`looping create`), previsualizador de interfaz SwiftUI en terminal TUI y transpilador nativo a Swift 5.9+.
* **Módulos Integrados del Ecosistema CS**: `csid` (OAuth 2.1 PKCE S256), `csms` (Cifrado E2EE con derivación SHA-256), `game_plus` (2D/3D) y `core_utils`.
* **Arranque Instantáneo**: Menos de **0.5 ms** en C++17 nativo sin dependencias de Node.js ni contenedores web.

Todas las versiones de arquitectura están catalogadas bajo la serie oficial (**3.0A**, **2.8A**, **2.7A**, **2.6A**, **2.5A**, **2.0A**, **1.0A**).

---

## 🚀 Instalación Rápida

### Opción 1: Vía Homebrew (Recomendado en macOS)
```bash
brew tap cokistudios/tap
brew install looping smiledev
brew install cokistudios/tap/smiledev-alpha   # Alpha! 1.1A (Super Experimental)
```

### Opción 2: Descarga de Binario Universal (Apple Silicon + Intel)
```bash
# Canal de Producción (Looping v1.1 / Arch 3.0A)
curl -fsSL https://repo.cokistudios.com/engine/install.sh | bash

# Canal RC (Omega Looping) o Super Experimental (Alpha! Looping)
curl -fsSL https://repo.cokistudios.com/engine/install.sh | bash -s -- --channel=omega
curl -fsSL https://repo.cokistudios.com/engine/install.sh | bash -s -- --channel=alpha
```
O descarga directa del tarball verificado:
```bash
curl -LO https://repo.cokistudios.com/engine/looping-v1.1-darwin-universal.tar.gz
tar -xzf looping-v1.1-darwin-universal.tar.gz
sudo mv looping smiledev /usr/local/bin/
```

### Opción 3: Compilación desde el Código Fuente
Requiere un compilador compatible con C++17 (`clang++` o `g++`):
```bash
git clone https://github.com/CokiStudios/engine.git
cd engine
make
make test
sudo make install
```

---

## 🕹️ Motor de Videojuegos 3D & 2D (Arch 3.0A)

Looping v1.1 incorpora directivas declarativas 3D de primera clase (`define game3d`, `scene3d.*`, `camera3d.set`, `light3d.*`, `physics3d.*`, `spawn3d.*`):

```loop
define game3d "CyberOdyssey3D" version 1.1:
    scene3d.world(name: "NeonCircuit3D", size: (180, 80, 180), sky: "cyber_night", fog: 0.010, grid: true)
    camera3d.set(mode: "chase", pos: (0, 8, -16), target: (0, 2, 10), fov: 64, smooth: 0.14)
    light3d.directional(dir: (-0.4, -1.0, -0.35), color: "#ffffff", intensity: 1.1)
    physics3d.config(gravity: -26.0, substeps: 4, friction: 0.86, bounce: 0.15)

    spawn3d.player("Pilot3D", at: (0, 2, 0), mesh: "hovercraft", speed: 24, jump: 16, color: "#38bdf8")
    spawn3d.box(at: (0, -0.5, 14), size: (14, 1.2, 28), color: "#6366f1")
    spawn3d.ring(at: (0, 4, 18), radius: 3.2, points: 250, boost: 22, color: "#00f5d4")
    spawn3d.enemy("Sentinel3D", at: (0, 2.5, 32), range: (-8, 8), speed: 9, health: 60)
    spawn3d.portal(at: (0, 3.5, 48), radius: 3.8, label: "OMEGA GATE")
```

Para previsualizar en viewport Z-Buffer TrueColor en terminal o ejecutar la demo completa:
```bash
./bin/looping preview3d samples/cyber_odyssey_3d.loop
./bin/looping run samples/cyber_odyssey_3d.loop
```

---

## 🌟 smiledev — Universal Package Manager

`smiledev` es el gestor oficial de paquetes, módulos y canales de actualización para Looping:

```bash
# Consultar versión y canal activo
smiledev version
smiledev version --omega
smiledev version --alpha

# Actualizar motor según el canal deseado
smiledev upgrade --channel=prod     # Looping & smiledev v1.1 (Producción)
smiledev upgrade --channel=omega    # Omega Looping (Release Candidate / RC)
smiledev upgrade --alpha            # Alpha! 1.1A (Super Experimental para smiledev & Looping)

# Instalar módulos desde el registro
smiledev add loop-math-3d
smiledev test
```

---

## 📂 Estructura del Repositorio

* [`cpp/`](cpp/): Código fuente nativo en C++17 (`Looping3D.hpp`, Lexer, Parser, AST, Compilador, Intérprete, REPL, SwiftUI TUI y Native Window).
* [`bin/`](bin/): Binarios precompilados universales (`looping`, `smiledev`).
* [`modules/`](modules/): Módulos estándar comunitarios e híbridos (`loop-math-3d` v2.0.0, `loop-game-plus` v2.0.0, `loop-core-utils`, `csid-login-button`, `loop-node-window`, `loop-gui-qt`, `loop-gui-tk`, etc.).
* [`samples/`](samples/): Ejemplos completos de videojuegos 3D y 2D (`cyber_odyssey_3d.loop`), dashboards SwiftUI, pipelines y algoritmos.
* [`c2/`](c2/): Especificación de interfaz declarativa C2 y perfiles de tema (`aqua_nomad_ui.c2`, `cyber_neon_xui.c2`, etc.).
* [`extensions/`](extensions/): Extensión oficial de Visual Studio Code (`vscode-looping-cs-core` v1.1.0) con visor `.c2` e inspector 2D/3D.
* [`web/`](web/): Portal de descargas, motor Web Core Canvas 2D/3D (`looping_core.js`) y el GUI oficial [`smiledev.html`](web/smiledev.html).
* [`legacy/`](legacy/): Archivo histórico del prototipo original en Node.js previo a la transición a C++17.
* [`debian/`](debian/): Metadatos de empaquetado para distribuciones Linux basadas en Debian / Ubuntu.
* [`docs/`](docs/): Especificación formal del lenguaje, canales de nomenclatura, `smiledev` y subsistema de módulos.
* [`Formula/`](Formula/): Fórmulas oficiales de Homebrew (`looping.rb`, `smiledev.rb`, `smiledev-alpha.rb`).
* [`dist/`](dist/): Paquetes universales de lanzamiento (`v1.1`, `v1.1A` Alpha! y `v3.0A`) y firmas criptográficas SHA-256.

---

## 🏛️ Historial de Versiones y Serie de Arquitectura

| Serie / Canal | Versión | Fecha Hito | Hitos Principales |
| :--- | :--- | :--- | :--- |
| **Looping & smiledev (Prod / Alpha! 1.1A)** | **v1.1 / 1.1A (Arch 3.0A)** | **11 Nov 2026** | **Producción v1.1 & Alpha! 1.1A para smiledev**. Modo terminal persistente (`-i` / `--persistent`), Apple Metal 4 (`metal4.*`, `240Hz ProMotion`), canal `Alpha! 1.1A`, Motor 3D Quantum (`Looping3D.hpp`, Z-Buffer, Raycast), módulos CS integrados y **71/71 tests C++ nativos**. |
| Producción | **v1.0.0 / Arch 2.8A** | 27 Oct 2026 | Primera versión de producción. Motor 2D Quantum, `loop-node-window` v2.0.0, 57/57 tests C++ nativos. |
| Arquitectura | **2.8A** | 26 Oct 2026 | SwiftUI TUI, Cripto CSMS (`crc32`, `constant_time_eq`), CSV y DateTime stdlib. |
| Arquitectura | **2.7A** | 19 Oct 2026 | Integración con Eco Hub, match multi-patrón `\|`, rangos `..=` y pipelines funcionales. |
| Arquitectura | **2.6A** | 19 Oct 2026 | Distribución Homebrew, empaquetador universal y primera integración de `smiledev`. |
| Arquitectura | **2.5A** | 04 Oct 2026 | Módulos GUI nativos (`loop-gui-tk`, `loop-gui-qt`) y soporte inicial para Felte Store. |
| Arquitectura | **2.0A / 2.2A** | **03 Oct 2026** | **Transición histórica de Node.js a C++17 nativo** (Commit `67a72391`). Arranque < 0.5 ms, RSS < 2.8 MB y binarios universales Mach-O. |
| Arquitectura | **1.0A** | Jul–Ago 2026 | Prototipo inicial interpretado en Node.js (`bin/looping.js`). |

---

## 📜 Licencia

Desarrollado con ❤️ por **Coki Studios** y **Holo Entertainment** © 2026.  
Distribuido bajo la Licencia **MIT**. Consulta el archivo [LICENSE](LICENSE) para más detalles.
