# See COPYING file for copyright, license and warranty details.

include config.mk

SRC = getgbook.c
OBJ = $(SRC:.c=.o)
BIN = $(SRC:.c=)

all: $(BIN)

$(OBJ): util.c

.c.o:
	@echo CC $<
	@$(CC) -c $(CFLAGS) $<

getgbook: $(OBJ)
	@echo LD $@
	@$(CC) -o $@ $(OBJ) $(LDFLAGS)

install: all
	cp -f $(BIN) $(DESTDIR)$(PREFIX)/bin

uninstall:
	cd $(DESTDIR)$(PREFIX)/bin && rm -f $(BIN)

clean:
	rm -f -- $(BIN) $(OBJ)

.PHONY: all clean install uninstall
