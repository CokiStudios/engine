# Looping Experimental Runtime (`Assembly + Objective-C ABI + C11`)

Micro-runtime experimental de **Looping** sin Python, sin Node.js y sin biblioteca estándar de C++:

- **`asm_core.S`**: Núcleo universal en ensamblador ARM64 (`Apple Silicon`) + x86_64 (`exp_asm_strlen`, `exp_asm_fnv1a64`, `exp_asm_clear_rgba32`, `exp_asm_fill_rect_rgba32`).
- **`objc_window.c`**: Puente puro en C11 hacia Cocoa (`NSApplication`, `NSWindow`, `NSView`, `NSTextField`) usando directamente `objc_msgSend` y CoreGraphics sobre el framebuffer de ensamblador.
- **`main.c`**: Mini-intérprete `.loop` y servidor HTTP nativo sobre sockets BSD POSIX (`--serve <port>`), sin dependencias externas para servir ni para abrir ventanas.

## Compilar y Probar

```bash
make -C experimental/asm-objc-c test
./experimental/asm-objc-c/looping-exp --serve 8787
```
