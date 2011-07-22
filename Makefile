# See COPYING file for copyright, license and warranty details.
include config.mk

NAME = getxbook

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

dist:
	@mkdir -p $(NAME)-$(VERSION)
	@cp $(SRC) util.h util.c Makefile config.mk COPYING $(NAME)-$(VERSION)
	@tar c $(NAME)-$(VERSION) | gzip -c > $(NAME)-$(VERSION).tar.gz
	@rm -rf $(NAME)-$(VERSION)
	@echo $(NAME)-$(VERSION).tar.gz

.PHONY: all clean install uninstall dist
