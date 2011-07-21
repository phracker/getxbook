# See COPYING file for copyright, license and warranty details.
include config.mk

SRC = getgbook.c
LIB = util.o

BIN = $(SRC:.c=)
OBJ = $(SRC:.c=.o) $(LIB)

all: $(BIN)

$(OBJ): util.h config.mk
$(BIN): util.a

.o:
	@echo LD $@
	@$(LD) -o $@ $< util.a $(LDFLAGS)

.c.o:
	@echo CC $<
	@$(CC) -c $(CFLAGS) $<

util.a: $(LIB)
	@echo AR $@
	@$(AR) -r -c $@ $(LIB)
	@ranlib $@

install: all
	cp -f $(BIN) $(DESTDIR)$(PREFIX)/bin

uninstall:
	cd $(DESTDIR)$(PREFIX)/bin && rm -f $(BIN)

clean:
	rm -f -- $(BIN) $(OBJ) util.a

.PHONY: all clean install uninstall
