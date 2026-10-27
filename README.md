# ⚡ Looping Engine & smiledev
> **Looping Commercial Production Edition v1.0.0 (Architecture 2.8A Quantum Signature)**  
> *The Dual Hybrid High-Performance Language & Game Engine by Coki Studios & Holo Entertainment*

[![Release](https://img.shields.io/badge/release-v1.0.0--commercial-38bdf8.svg?style=flat-square)](https://github.com/CokiStudios/engine/releases/tag/v1.0.0-commercial)
[![Architecture](https://img.shields.io/badge/arch-2.8A%20Quantum-10b981.svg?style=flat-square)](https://cokistudios.github.io/developer.html)
[![Tests](https://img.shields.io/badge/tests-57%2F57%20passing-a855f7.svg?style=flat-square)](https://github.com/CokiStudios/engine)
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
* [`web/`](web/): Motor de ejecución y renderizado 2D para la web y navegadores (`looping_core.js`).
* [`bin/`](bin/): Binarios precompilados universales (`looping`, `smiledev`).
* [`modules/`](modules/): Módulos estándar comunitarios (`loop-core-utils`, `loop-node-window`, `loop-gui-qt`, `loop-gui-tk`).
* [`samples/`](samples/): Ejemplos completos de videojuegos, dashboards SwiftUI, pipelines y algoritmos.
* [`Formula/`](Formula/): Fórmulas oficiales de Homebrew (`looping.rb`, `smiledev.rb`).
* [`SPECIFICATION.md`](SPECIFICATION.md): Especificación exhaustiva de sintaxis, palabras clave y stdlib.

---

## 🏛️ Historial de Versiones de Arquitectura

| Serie | Versión | Hitos Principales |
| :--- | :--- | :--- |
| **Comercial** | **v1.0.0** | **Primera versión comercial de producción**. Motor 2D Quantum, `loop-node-window` v2.0.0, 57/57 tests C++ nativos. |
| Arquitectura | **2.8A** | Novedades Quantum Signature: SwiftUI TUI, Cripto CSMS v2.8 (`crc32`, `constant_time_eq`), CSV y DateTime stdlib. |
| Arquitectura | **2.7A** | Integración con Eco Hub, match multi-patrón `\|`, rangos `..=` y pipelines funcionales. |
| Arquitectura | **2.6A** | Distribución Homebrew, empaquetador universal y primera integración de `smiledev`. |
| Arquitectura | **2.5A** | Módulos GUI nativos (`loop-gui-tk`, `loop-gui-qt`) y soporte inicial para Felte Store. |
| Arquitectura | **2.0A** | Introducción de directivas `@app`, `ui.*` y runtime híbrido Web + POSIX. |
| Arquitectura | **1.0A** | Nacimiento de Looping como lenguaje minimalista para el ecosistema Coki Studios. |

---

## 📜 Licencia

Desarrollado con ❤️ por **Coki Studios** y **Holo Entertainment** © 2026.  
Distribuido bajo la Licencia **MIT**. Consulta el archivo [LICENSE](LICENSE) para más detalles.
