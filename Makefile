PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin

all: build

build:
	$(MAKE) -C cpp all

test: build
	./bin/looping test
	./bin/smiledev version

install:
	@mkdir -p $(DESTDIR)$(BINDIR)
	install -m 755 bin/looping $(DESTDIR)$(BINDIR)/looping
	install -m 755 bin/smiledev $(DESTDIR)$(BINDIR)/smiledev
	ln -sf $(DESTDIR)$(BINDIR)/looping $(DESTDIR)$(BINDIR)/ruuping
	@echo "Installed looping and smiledev to $(DESTDIR)$(BINDIR)"

clean:
	$(MAKE) -C cpp clean

.PHONY: all build test install clean
