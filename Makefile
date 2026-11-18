PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

ifeq ($(OS),Windows_NT)
	EXE := .exe
else
	EXE :=
	UNAME_S := $(shell uname -s)
endif

all: build

build:
	$(MAKE) -C cpp all
ifeq ($(UNAME_S),Darwin)
	$(MAKE) -C experimental/asm-objc-c all
endif

wasm:
	./scripts/build-wasm.sh

test: build
ifeq ($(OS),Windows_NT)
	bin/looping.exe test
	powershell -NoProfile -ExecutionPolicy Bypass -File bin/smiledev.ps1 version
else
	./bin/looping test
ifeq ($(UNAME_S),Darwin)
	./bin/looping-exp --test
endif
	./bin/smiledev version
endif

install:
ifeq ($(OS),Windows_NT)
	powershell -NoProfile -ExecutionPolicy Bypass -File install.ps1
else
	@mkdir -p $(DESTDIR)$(BINDIR)
	install -m 755 bin/looping $(DESTDIR)$(BINDIR)/looping
	@if [ -f bin/looping-exp ]; then install -m 755 bin/looping-exp $(DESTDIR)$(BINDIR)/looping-exp; fi
	install -m 755 bin/smiledev $(DESTDIR)$(BINDIR)/smiledev
	ln -sf $(DESTDIR)$(BINDIR)/looping $(DESTDIR)$(BINDIR)/ruuping
	@echo "Installed looping and smiledev to $(DESTDIR)$(BINDIR)"
endif

clean:
	$(MAKE) -C cpp clean
ifeq ($(UNAME_S),Darwin)
	$(MAKE) -C experimental/asm-objc-c clean
endif

.PHONY: all build wasm test install clean
