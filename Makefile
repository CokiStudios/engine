PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

ifeq ($(OS),Windows_NT)
	EXE := .exe
else
	EXE :=
endif

all: build

build:
	$(MAKE) -C cpp all

wasm:
	./scripts/build-wasm.sh

test: build
ifeq ($(OS),Windows_NT)
	bin/looping.exe test
	powershell -NoProfile -ExecutionPolicy Bypass -File bin/smiledev.ps1 version
else
	./bin/looping test
	./bin/smiledev version
endif

install:
ifeq ($(OS),Windows_NT)
	powershell -NoProfile -ExecutionPolicy Bypass -File install.ps1
else
	@mkdir -p $(DESTDIR)$(BINDIR)
	install -m 755 bin/looping $(DESTDIR)$(BINDIR)/looping
	install -m 755 bin/smiledev $(DESTDIR)$(BINDIR)/smiledev
	ln -sf $(DESTDIR)$(BINDIR)/looping $(DESTDIR)$(BINDIR)/ruuping
	@echo "Installed looping and smiledev to $(DESTDIR)$(BINDIR)"
endif

clean:
	$(MAKE) -C cpp clean

.PHONY: all build wasm test install clean
