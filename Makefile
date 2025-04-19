# This file is hacky, please pr with a more elegant solution
PREFIX=/usr/local/bin
BUILDDIR=bin
EXEBIN=alm
SROUCEDIR=src
CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb
LIBS=

build:
	@$(CC) $(DEBUG) $(CFLAGS) -o $(BUILDDIR)/$(EXEBIN) $(SROUCEDIR)/* $(LIBS)

debug: DEBUG = -DDEBUG -ggdb
debug: build

.PHONY: install
install:
	sudo mkdir -p $(PREFIX)
	sudo cp $(BUILDDIR)/$(EXEBIN) $(PREFIX)/$(EXEBIN)

.PHONY: uninstall
uninstall:
	sudo rm -rf $(PREFIX)/$(EXEBIN)

clean:
	rm -rf $(BUILDDIR)
