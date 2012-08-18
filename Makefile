# See COPYING file for copyright and license details.
include config.mk

NAME = getxbook

SRC = getgbook.c getabook.c getbnbook.c
LIB = util.o
GUI = getxbookgui.tcl
DOC = README COPYING INSTALL LEGAL
EXTRAS = extras/mkpdf.sh extras/mkocrpdf.sh extras/mkdjvu.sh extras/mkocrtxt.sh extras/mkocrdjvu.sh

BIN = $(SRC:.c=)
MAN = $(SRC:.c=.1) $(GUI:.tcl=.1) $(NAME).1
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
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(BIN) $(DESTDIR)$(PREFIX)/bin
	sed "s:^set iconpath .*:set iconpath \"$(DESTDIR)$(PREFIX)/share/pixmaps\":" < $(GUI) \
	    > $(DESTDIR)$(PREFIX)/bin/$(GUI:.tcl=)
	chmod +x $(DESTDIR)$(PREFIX)/bin/getxbookgui
	mkdir -p $(DESTDIR)$(PREFIX)/share/pixmaps
	cp icons/* $(DESTDIR)$(PREFIX)/share/pixmaps/
	mkdir -p $(DESTDIR)$(PREFIX)/share/applications
	cp $(NAME).desktop $(DESTDIR)$(PREFIX)/share/applications/
	mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	for f in $(MAN); do sed "s/VERSION/$(VERSION)/g" < $$f > $(DESTDIR)$(MANPREFIX)/man1/$$f; done

uninstall:
	cd $(DESTDIR)$(PREFIX)/bin && rm -f $(BIN) $(GUI:.tcl=)
	cd $(DESTDIR)$(MANPREFIX)/man1 && rm -f $(MAN)
	rm -rf $(DESTDIR)$(PREFIX)/share/$(NAME)

clean:
	rm -f -- $(BIN) $(OBJ) util.a index.html comparison.html

dist:
	mkdir -p $(NAME)-$(VERSION)
	cp $(SRC) $(GUI) $(DOC) $(MAN) util.h util.c Makefile config.mk $(NAME)-$(VERSION)
	mkdir -p $(NAME)-$(VERSION)/icons
	cp icons/* $(NAME)-$(VERSION)/icons/
	mkdir -p $(NAME)-$(VERSION)/extras
	cp $(EXTRAS) $(NAME)-$(VERSION)/extras/
	tar c $(NAME)-$(VERSION) | bzip2 -c > $(NAME)-$(VERSION).tar.bz2
	gpg -b < $(NAME)-$(VERSION).tar.bz2 > $(NAME)-$(VERSION).tar.bz2.sig
	rm -rf $(NAME)-$(VERSION)
	echo $(NAME)-$(VERSION).tar.bz2 $(NAME)-$(VERSION).tar.bz2.sig

# needs config.mk set up to build statically
dist-static: $(BIN)
	mkdir -p $(NAME)-$(VERSION)
	cp $(BIN) $(GUI) $(DOC) $(MAN) config.mk $(NAME)-$(VERSION)
	sed 's/^install: all$$/install:/' < Makefile > $(NAME)-$(VERSION)/Makefile
	mkdir -p $(NAME)-$(VERSION)/icons
	cp icons/* $(NAME)-$(VERSION)/icons/
	mkdir -p $(NAME)-$(VERSION)/extras
	cp $(EXTRAS) $(NAME)-$(VERSION)/extras/
	tar c $(NAME)-$(VERSION) | bzip2 -c > $(NAME)-$(VERSION)-static.tar.bz2
	gpg -b < $(NAME)-$(VERSION)-static.tar.bz2 > $(NAME)-$(VERSION)-static.tar.bz2.sig
	rm -rf $(NAME)-$(VERSION)
	echo $(NAME)-$(VERSION)-static.tar.bz2 $(NAME)-$(VERSION)-static.tar.bz2.sig

getxbookgui.exe: getxbookgui.tcl
	@echo STARPACK $@
	@sdx qwrap getxbookgui.tcl
	@sdx unwrap getxbookgui.kit
	@sdx wrap $@ -runtime $(W32TCLKIT)
	@rm -r getxbookgui.kit getxbookgui.vfs

# needs to be run from a mingw setup
dist-win: $(BIN) $(GUI:.tcl=.exe)
	mkdir -p $(NAME)-win
	cp $(OBJ:.o=.exe) $(GUI:.tcl=.exe) $(NAME)-win
	mkdir -p $(NAME)-win/icons
	cp icons/* $(NAME)-win/icons/
	for f in LEGAL README COPYING; do \
	sed 's/$$/\r/g' < $$f > $(NAME)-win/$$f.txt; done
	for f in *1; do \
	b=`basename $$f .1`; \
	groff -m man -T utf8 < $$f | col -bx | sed 's/$$/\r/g' > $(NAME)-win/$$b.txt; done
	zip -j $(NAME)-$(VERSION)-win.zip $(NAME)-win/*
	gpg -b < $(NAME)-$(VERSION)-win.zip > $(NAME)-$(VERSION)-win.zip.sig
	rm -rf $(NAME)-win
	echo $(NAME)-$(VERSION)-win.zip $(NAME)-$(VERSION)-win.zip.sig

# needs to be run from a mac
# currently unused thanks to kind homebrew people packaging it
dist-mac: $(BIN) $(GUI)
	mkdir -p $(NAME)-$(VERSION)/$(NAME).app/Contents/MacOS
	mkdir -p $(NAME)-$(VERSION)/$(NAME).app/Contents/Resources
	cp $(BIN) $(NAME)-$(VERSION)/$(NAME).app/Contents/MacOS/
	cp $(GUI) $(NAME)-$(VERSION)/$(NAME).app/Contents/MacOS/$(GUI:.tcl=)
	for f in $(DOC); do cp $$f $(NAME)-$(VERSION)/$$f.txt; done
	echo '<?xml version="1.0" encoding="UTF-8"?><!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd"><plist version="1.0"><dict><key>CFBundlePackageType</key><string>APPL</string>'"<key>CFBundleExecutable</key><string>getxbookgui</string><key>CFBundleVersion</key><string>$(VERSION)</string><key>CFBundleName</key><string>$(NAME)</string></dict></plist>" > $(NAME)-$(VERSION)/$(NAME).app/Contents/Info.plist
	hdiutil create -srcfolder $(NAME) $(NAME)-$(VERSION).dmg
	hdiutil internet-enable -yes $(NAME)-$(VERSION).dmg
	gpg -b < $(NAME)-$(VERSION)-mac.dmg > $(NAME)-$(VERSION)-mac.dmg.sig
	rm -rf $(NAME)-$(VERSION)
	echo $(NAME)-$(VERSION)-mac.dmg $(NAME)-$(VERSION)-mac.dmg.sig

index.html: doap.ttl README
	echo making webpage
	echo "<!DOCTYPE html><html><head><title>$(NAME)</title>" > $@
	echo '<link rel="alternate" type="text/turtle" title="rdf" href="doap.ttl" />' >> $@
	echo '<style type="text/css">' >> $@
	echo "body {font-family:sans-serif; width:38em; margin:auto; max-width:94%;}" >> $@
	echo "h1 {font-size:1.6em; text-align:center;}" >> $@
	echo "a {text-decoration:none; border-bottom:thin dotted;}" >> $@
	echo "img {margin: auto; border: thin solid; display: block;}" >> $@
	echo "</style></head><body>" >> $@
	sed '5q' < README | smu >> $@
	echo "<p><img src="screenshot1.png" alt="screenshot"/></p>" >> $@
	echo "<h2>download</h2>" >> $@
	echo "[$(NAME) $(VERSION) source]($(NAME)-$(VERSION).tar.bz2) ([sig]($(NAME)-$(VERSION).tar.bz2.sig)) ($(RELDATE))" | smu >> $@
	echo "[$(NAME) $(VERSION) linux static]($(NAME)-$(VERSION)-static.tar.bz2) ([sig]($(NAME)-$(VERSION)-static.tar.bz2.sig)) ($(RELDATE))" | smu >> $@
	echo "$(NAME) for mac os x [available from Homebrew](http://mxcl.github.com/homebrew/)" | smu >> $@
	#echo "[$(NAME) $(VERSION) windows]($(NAME)-$(VERSION)-win.zip) ([sig]($(NAME)-$(VERSION)-win.zip.sig)) ($(RELDATE))" | smu >> $@
	sed '1,5d' < README | smu >> $@
	echo "[comparison with similar programs](comparison)" | smu >> $@
	echo '<hr />' >> $@
	sh websummary.sh doap.ttl | smu >> $@
	echo '</body></html>' >> $@

comparison.html: comparison.txt
	echo making comparison webpage
	echo "<!DOCTYPE html><html><head><title>$(NAME)</title>" > $@
	echo '<link rel="alternate" type="text/turtle" title="rdf" href="doap.ttl" />' >> $@
	echo '<style type="text/css">' >> $@
	echo "body {font-family:sans-serif; width:38em; margin:auto; max-width:94%;}" >> $@
	echo "h1 {font-size:1.6em; text-align:center;}" >> $@
	echo "a {text-decoration:none; border-bottom:thin dotted;}" >> $@
	echo "img {margin: auto; border: thin solid; display: block;}" >> $@
	echo "</style></head><body>" >> $@
	smu < comparison.txt >> $@
	echo '</body></html>' >> $@

.PHONY: all clean install uninstall dist dist-win dist-mac
.SILENT: index.html comparison.html dist
