OBJDIR	=
PREFIX	= /usr/local
DESTDIR	=
MKDIR	= mkdir -m 0755 -p
INSTALL	= install
RM	= rm -f


all:

clean:

distclean: clean

install: all
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/data
	$(INSTALL) -m 0644 data/pkgconfig.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/data/pkgconfig.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/doc
	$(INSTALL) -m 0644 doc/docbook.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/doc/docbook.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/doc
	$(INSTALL) -m 0644 doc/gtkdoc.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/doc/gtkdoc.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/doc
	$(INSTALL) -m 0644 doc/manual.css.xml $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/doc/manual.css.xml
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/doc
	$(INSTALL) -m 0644 doc/markdown.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/doc/markdown.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/po
	$(INSTALL) -m 0644 po/gettext.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/po/gettext.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tests
	$(INSTALL) -m 0644 tests/clint.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tests/clint.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tests
	$(INSTALL) -m 0644 tests/coverage.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tests/coverage.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tests
	$(INSTALL) -m 0644 tests/fixme.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tests/fixme.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tests
	$(INSTALL) -m 0644 tests/htmllint.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tests/htmllint.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tests
	$(INSTALL) -m 0644 tests/phplint.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tests/phplint.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tests
	$(INSTALL) -m 0644 tests/pylint.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tests/pylint.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tests
	$(INSTALL) -m 0644 tests/shlint.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tests/shlint.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tests
	$(INSTALL) -m 0644 tests/xmllint.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tests/xmllint.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tools
	$(INSTALL) -m 0644 tools/appbroker.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tools/appbroker.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tools
	$(INSTALL) -m 0644 tools/platform.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tools/platform.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tools
	$(INSTALL) -m 0644 tools/subst.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tools/subst.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tools
	$(INSTALL) -m 0644 tools/template.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tools/template.sh

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/data/pkgconfig.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/doc/docbook.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/doc/gtkdoc.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/doc/manual.css.xml
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/doc/markdown.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/po/gettext.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tests/clint.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tests/coverage.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tests/fixme.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tests/htmllint.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tests/phplint.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tests/pylint.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tests/shlint.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tests/xmllint.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tools/appbroker.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tools/platform.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tools/subst.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/tools/template.sh

.PHONY: all clean distclean install uninstall
