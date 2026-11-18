CC ?= clang
CFLAGS ?= -std=c11 -O3 -Wall -Wextra
ARCH_FLAGS ?= -arch arm64 -arch x86_64
LDFLAGS = -framework Cocoa -framework CoreGraphics -lobjc

TARGET = looping-exp
BIN_TARGET = ../../bin/looping-exp

all: $(TARGET) $(BIN_TARGET)

$(TARGET): asm_core.S objc_window.c main.c
	@rm -f $(TARGET)
	$(CC) $(CFLAGS) $(ARCH_FLAGS) asm_core.S objc_window.c main.c $(LDFLAGS) -o $(TARGET)
	@if command -v codesign >/dev/null 2>&1; then codesign -s - --force $(TARGET) >/dev/null 2>&1 || true; fi
	@ls -lh $(TARGET)

$(BIN_TARGET): $(TARGET)
	@mkdir -p ../../bin
	@rm -f $(BIN_TARGET)
	@cp $(TARGET) $(BIN_TARGET)
	@chmod 755 $(BIN_TARGET)
	@if command -v codesign >/dev/null 2>&1; then codesign -s - --force $(BIN_TARGET) >/dev/null 2>&1 || true; fi

test: all
	./$(TARGET) --test

clean:
	rm -f $(TARGET) $(BIN_TARGET)

.PHONY: all test clean
