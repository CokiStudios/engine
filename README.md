# ⚡ Looping Engine & smiledev
> **Looping Commercial Production Edition v1.0.0 (Architecture 2.8A Quantum Signature)**  
> *The Dual Hybrid High-Performance Language & Game Engine by Coki Studios & Holo Entertainment*

[![Release](https://img.shields.io/badge/release-v1.0.0--commercial-38bdf8.svg?style=flat-square)](https://github.com/CokiStudios/engine/releases/tag/v1.0.0-commercial)
[![Architecture](https://img.shields.io/badge/arch-2.8A%20Quantum-10b981.svg?style=flat-square)](https://cokistudios.github.io/developer.html)
[![Tests](https://img.shields.io/badge/tests-58%2F58%20passing-a855f7.svg?style=flat-square)](https://github.com/CokiStudios/engine)
[![License](https://img.shields.io/badge/license-MIT-f59e0b.svg?style=flat-square)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-macOS%20Universal%20%7C%20Linux%20%7C%20POSIX-64748b.svg?style=flat-square)](https://github.com/CokiStudios/engine)

---

## 🌟 Visión General

**Looping** (`.loop` / `.ruup`) es un lenguaje de programación moderno, tipado dinámico y de altísimo rendimiento diseñado para la creación ágil de:
* **Videojuegos 2D con Física Cuántica**: Motor a 60 FPS con coyote time, jump buffering, dash con estelas, pisoteo de enemigos, plataformas móviles maglev, sintetizador procedural Web Audio y controles táctiles/gamepad.
* **Aplicaciones Nativas & Declarativas**: Sintaxis de interfaz reactiva, previsualizador de interfaz SwiftUI en terminal TUI y transpilador nativo a Swift 5.9+.
* **Criptografía & Seguridad CSMS**: SHA-256, HMAC-SHA256, CRC-32, UUID v4, comparación en tiempo constante y derivación de salas seguras.
* **Ciencia de Datos & Pipelines**: Álgebra vectorial (norma, dot, linspace), percentiles, agregaciones y tuberías funcionales con operador `|>`.
* **Arranque Instantáneo**: Menos de **0.5 ms** en C++17 nativo sin dependencias de Node.js ni contenedores web.

Todas las versiones históricas previas han sido consolidadas bajo la serie de arquitectura (**2.8A**, **2.7A**, **2.6A**, **2.5A**, **2.0A**, **1.0A**), dando paso a esta **1ª Versión Comercial Oficial**.

---

## 🚀 Instalación Rápida

### Opción 1: Vía Homebrew (Recomendado en macOS)
```bash
brew tap cokistudios/tap
brew install looping smiledev
```

### Opción 2: Descarga de Binario Universal (Apple Silicon + Intel)
```bash
curl -fsSL https://repo.cokistudios.com/engine/install.sh | bash
```
O descarga directa del tarball verificado:
```bash
curl -LO https://repo.cokistudios.com/engine/looping-v1.0.0-commercial-darwin-universal.tar.gz
tar -xzf looping-v1.0.0-commercial-darwin-universal.tar.gz
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

## 🎮 Motor de Videojuegos 2D

Looping incluye un motor cuántico 2D integrado tanto en la consola POSIX como en Web y en el runtime [`loop-node-window`](modules/loop-node-window):

```loop
define game "CyberRunner2D" version 1.0:
    game.gravity = 980.0
    game.world.bounds = [0, 0, 3200, 600]
    game.camera.smooth = 0.12

    spawn.player "Pilot" at [100, 480]:
        speed = 260
        jump_force = 440
        double_jump = true
        dash = true

    spawn.moving_platform "MaglevA" at [500, 380]:
        axis = "x"
        min_pos = 450
        max_pos = 780
        speed = 90

    spawn.enemy "Drone" at [950, 480]:
        patrol = [900, 1150]
        stompable = true

    spawn.portal "VictoryPortal" at [2800, 450]
```

Para probar el juego de demostración interactivo con `loop-node-window`:
```bash
./bin/looping run samples/quantum_2d_game_nodewindow.loop
```

---

## 🌟 smiledev — Universal Package Manager

`smiledev` es el gestor oficial de paquetes y módulos para el ecosistema Looping:

```bash
# Consultar versión
smiledev version

# Inicializar un nuevo proyecto
smiledev init MiNuevoProyecto

# Instalar módulos desde el registro en la nube
smiledev add loop-core-utils

# Probar el proyecto
smiledev test

# Enlazar un módulo local para desarrollo
smiledev link
```

---

## 📂 Estructura del Repositorio

* [`cpp/`](cpp/): Código fuente nativo en C++17 (Lexer, Parser, AST, Compilador, Intérprete, REPL, TUI SwiftUI y Native Window).
* [`bin/`](bin/): Binarios precompilados universales (`looping`, `smiledev`).
* [`modules/`](modules/): Módulos estándar comunitarios e híbridos (`loop-core-utils`, `csid-login-button`, `loop-game-plus`, `loop-node-window`, `loop-gui-qt`, `loop-gui-tk`, etc.).
* [`samples/`](samples/): Ejemplos completos de videojuegos 2D, dashboards SwiftUI, pipelines y algoritmos.
* [`c2/`](c2/): Especificación de interfaz declarativa C2 y perfiles de tema (`aqua_nomad_ui.c2`, `cyber_neon_xui.c2`, etc.).
* [`extensions/`](extensions/): Extensión oficial de Visual Studio Code (`vscode-looping-cs-core`) con resaltado de sintaxis y autocompletado.
* [`web/`](web/): Portal de descargas, motor Web Core Canvas y el GUI oficial [`smiledev.html`](web/smiledev.html).
* [`legacy/`](legacy/): Archivo histórico del prototipo original en Node.js previo a la transición a C++17.
* [`debian/`](debian/): Metadatos de empaquetado para distribuciones Linux basadas en Debian / Ubuntu.
* [`docs/`](docs/): Especificación formal del lenguaje, de `smiledev` y del subsistema de módulos.
* [`Formula/`](Formula/): Fórmulas oficiales de Homebrew (`looping.rb`, `smiledev.rb`).
* [`dist/`](dist/): Paquetes universales de lanzamiento y firmas criptográficas SHA-256.
* [`scripts/`](scripts/): Herramientas de empaquetado y automatización de releases.
* [`assets/`](assets/): Iconos e identidad visual oficial de Looping y Shine Launcher.
* [`install.sh`](install.sh): Script de instalación automatizada para entornos POSIX.

---

## 🏛️ Historial de Versiones de Arquitectura

| Serie | Versión | Fecha Hito | Hitos Principales |
| :--- | :--- | :--- | :--- |
| **Comercial** | **v1.0.0** | **27 Oct 2026** | **Primera versión comercial de producción**. Motor 2D Quantum, `loop-node-window` v2.0.0, 57/57 tests C++ nativos. |
| Arquitectura | **2.8A** | 26 Oct 2026 | Novedades Quantum Signature: SwiftUI TUI, Cripto CSMS v2.8 (`crc32`, `constant_time_eq`), CSV y DateTime stdlib. |
| Arquitectura | **2.7A** | 19 Oct 2026 | Integración con Eco Hub, match multi-patrón `\|`, rangos `..=` y pipelines funcionales. |
| Arquitectura | **2.6A** | 19 Oct 2026 | Distribución Homebrew, empaquetador universal y primera integración de `smiledev`. |
| Arquitectura | **2.5A** | 04 Oct 2026 | Módulos GUI nativos (`loop-gui-tk`, `loop-gui-qt`) y soporte inicial para Felte Store. |
| Arquitectura | **2.0A / 2.2A** | **03 Oct 2026** | **Transición histórica de Node.js a C++17 nativo** (Commit `67a72391`). Eliminación de V8/Node, arranque sub-milisegundo (< 0.5 ms), memoria RSS < 3 MB y binarios universales Mach-O. |
| Arquitectura | **1.0A** | Jul–Ago 2026 | Prototipo inicial interpretado en Node.js (`bin/looping.js`) para experimentación de sintaxis básica. |

---

## ⚡ Transición Histórica: Del Prototipo Node.js a C++17 Nativo

### ¿Cuándo y cómo ocurrió la transición?
El hito definitivo de migración tuvo lugar el **3 de octubre de 2026** (Commit `67a72391`), fecha en que el equipo de desarrollo de Coki Studios y Holo Entertainment reemplazó íntegramente el prototipo original interpretado en JavaScript / Node.js (`bin/looping.js`) por un **motor de alto rendimiento escrito en C++17 moderno** (`cpp/`):

1. **La Fase de Prototipado en Node.js (Serie 1.0A)**:
   * Looping nació originalmente como un prototipo experimental en JavaScript para iterar rápidamente sobre la gramática y el diseño del lenguaje.
   * **Inconvenientes identificados**: El arranque sobre el motor V8 de Node.js tomaba entre **80 ms y 150 ms**, consumía más de **45 MB de memoria RSS** en reposo y requería un entorno Node.js instalado en el sistema host, impidiendo su despliegue liviano en consolas dedicadas (Shine Loop) y en el sistema operativo Holo Looping OoS.

2. **La Reescritura Completa en C++17 (3 de Octubre de 2026 — Serie 2.0A / 2.2A)**:
   * El **3 de octubre de 2026**, se completó la reescritura total del lexer, parser recursivo descendente, árbol AST e intérprete en **C++17 estándar**.
   * **Resultados técnicos inmediatos**:
     * ⏱️ **Tiempo de Inicio**: De ~120 ms se redujo a **< 0.5 ms** (>240x más veloz).
     * 💾 **Consumo de Memoria**: De 45 MB a **< 2.8 MB RSS**.
     * 🍏 **Binarios Universales Nativos**: Compilados directamente para macOS Apple Silicon (`arm64`) e Intel (`x86_64`), así como binarios ELF para Linux, sin requerir Node.js ni paquetes npm en tiempo de ejecución.
     * 🛡️ **Cero Contenedores Web en Herramientas del Sistema**: Alineación estricta con las políticas de arquitectura y seguridad de Coki Studios.

3. **Arquitectura Híbrida C++ / Web Core**:
   * **Entorno POSIX / CLI**: Ejecución 100% nativa en C++17 (`./bin/looping`).
   * **Entorno Web / Navegador**: Para el portal interactivo y el IDE hiOP, se desarrolló el motor gráfico Canvas acelerado [`web/looping_core.js`](web/looping_core.js), que replica la misma semántica del lenguaje sin requerir Node.js en el cliente.

---

## 📜 Licencia

Desarrollado con ❤️ por **Coki Studios** y **Holo Entertainment** © 2026.  
Distribuido bajo la Licencia **MIT**. Consulta el archivo [LICENSE](LICENSE) para más detalles.
