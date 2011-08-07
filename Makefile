# See COPYING file for copyright and license details.
include config.mk

NAME = getxbook

SRC = getgbook.c
LIB = util.o
SCRIPTS = getgmissing.sh getgfailed.sh makebookpdf.sh
DOC = README COPYING LEGAL

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
	cp -f $(BIN) $(SCRIPTS) $(DESTDIR)$(PREFIX)/bin

uninstall:
	cd $(DESTDIR)$(PREFIX)/bin && rm -f $(BIN) $(SCRIPTS)

clean:
	rm -f -- $(BIN) $(OBJ) util.a index.html

dist:
	mkdir -p $(NAME)-$(VERSION)
	cp $(SRC) $(SCRIPTS) $(DOC) util.h util.c Makefile config.mk $(NAME)-$(VERSION)
	tar c $(NAME)-$(VERSION) | gzip -c > $(NAME)-$(VERSION).tar.gz
	gpg -b < $(NAME)-$(VERSION).tar.gz > $(NAME)-$(VERSION).tar.gz.sig
	rm -rf $(NAME)-$(VERSION)
	echo $(NAME)-$(VERSION).tar.gz $(NAME)-$(VERSION).tar.gz.sig

index.html: doap.ttl README
	echo making webpage
	echo "<!DOCTYPE html><html><head><title>$(NAME)</title>" > $@
	echo '<link rel="alternate" type="text/turtle" title="rdf" href="doap.ttl" />' >> $@
	echo '<style type="text/css">' >> $@
	echo "body {font-family:sans-serif; width:38em; margin:auto; max-width:94%;}" >> $@
	echo "h1 {font-size:1.6em; text-align:center;}" >> $@
	echo "a {text-decoration:none; border-bottom-width:thin; border-bottom-style:dotted;}" >> $@
	echo "</style></head><body>" >> $@
	smu < README >> $@
	echo "<h2>download</h2>" >> $@
	echo "[$(NAME) $(VERSION)]($(NAME)-$(VERSION).tar.gz) ([sig]($(NAME)-$(VERSION).tar.gz.sig))" | smu >> $@
	echo '<hr />' >> $@
	sh websummary.sh doap.ttl | smu >> $@
	echo '</body></html>' >> $@

.PHONY: all clean install uninstall dist
.SILENT: index.html dist
