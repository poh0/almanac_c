# This file is hacky, please pr with a more elegant solution

PREFIX=/usr/local/bin
BUILDDIR=bin
EXEBIN=alm

CFLAGS=-Wall -Wextra -std=c11 -pedantic -ggdb
LIBS=

build:
	$(CC) $(CFLAGS) -o $(BUILDDIR)/$(EXEBIN) main.c $(LIBS)

test:
	@$(CC) $(CFLAGS) -o $(BUILDDIR)/$(EXEBIN) main.c $(LIBS)

.PHONY: install
install:
	sudo mkdir -p $(PREFIX)
	sudo cp $(BUILDDIR)/$(EXEBIN) $(PREFIX)/$(EXEBIN)

.PHONY: uninstall
uninstall:
	sudo rm -rf $(PREFIX)/$(EXEBIN)

clean:
	rm -rf $(BUILDDIR)
