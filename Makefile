# See COPYING file for copyright and license details.
include config.mk

NAME = getxbook

SRC = getgbook.c getabook.c
LIB = util.o
GUI = getxbookgui.tcl
DOC = README COPYING INSTALL LEGAL
EXTRAS = extras/mkpdf.sh extras/mkocrpdf.sh

BIN = $(SRC:.c=)
MAN = $(SRC:.c=.1)
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

getxbookgui.exe: getxbookgui.tcl
	@echo STARPACK $@
	@sdx qwrap getxbookgui.tcl
	@sdx unwrap getxbookgui.kit
	@sdx wrap $@ -runtime $(W32TCLKIT)
	@rm -r getxbookgui.kit getxbookgui.vfs

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(BIN) $(GUI) $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	for f in $(MAN); do sed "s/VERSION/$(VERSION)/g" < $$f > $(DESTDIR)$(MANPREFIX)/man1/$$f; done

uninstall:
	cd $(DESTDIR)$(PREFIX)/bin && rm -f $(BIN) $(GUI)
	cd $(DESTDIR)$(MANPREFIX)/man1 && rm -f $(MAN)

clean:
	rm -f -- $(BIN) $(OBJ) util.a index.html

dist:
	mkdir -p $(NAME)-$(VERSION)
	cp $(SRC) $(GUI) $(EXTRAS) $(DOC) util.h util.c Makefile config.mk $(NAME)-$(VERSION)
	tar c $(NAME)-$(VERSION) | bzip2 -c > $(NAME)-$(VERSION).tar.bz2
	gpg -b < $(NAME)-$(VERSION).tar.bz2 > $(NAME)-$(VERSION).tar.bz2.sig
	rm -rf $(NAME)-$(VERSION)
	echo $(NAME)-$(VERSION).tar.bz2 $(NAME)-$(VERSION).tar.bz2.sig

index.html: doap.ttl README
	echo making webpage
	echo "<!DOCTYPE html><html><head><title>$(NAME)</title>" > $@
	echo '<link rel="alternate" type="text/turtle" title="rdf" href="doap.ttl" />' >> $@
	echo '<style type="text/css">' >> $@
	echo "body {font-family:sans-serif; width:38em; margin:auto; max-width:94%;}" >> $@
	echo "h1 {font-size:1.6em; text-align:center;}" >> $@
	echo "a {text-decoration:none; border:thin dotted;}" >> $@
	echo "</style></head><body>" >> $@
	smu < README >> $@
	echo "<h2>download</h2>" >> $@
	echo "[$(NAME) $(VERSION)]($(NAME)-$(VERSION).tar.bz2) ([sig]($(NAME)-$(VERSION).tar.bz2.sig)) ($(RELDATE))" | smu >> $@
	echo '<hr />' >> $@
	sh websummary.sh doap.ttl | smu >> $@
	echo '</body></html>' >> $@

.PHONY: all clean install uninstall dist
.SILENT: index.html dist
