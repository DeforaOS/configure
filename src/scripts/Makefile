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
	$(MKDIR) $(DESTDIR)$(PREFIX)/libexec/configure/scripts/data
	$(INSTALL) -m 0755 data/pkgconfig.sh $(DESTDIR)$(PREFIX)/libexec/configure/scripts/data/pkgconfig.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/libexec/configure/scripts/doc
	$(INSTALL) -m 0755 doc/docbook.sh $(DESTDIR)$(PREFIX)/libexec/configure/scripts/doc/docbook.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/libexec/configure/scripts/doc
	$(INSTALL) -m 0755 doc/gtkdoc.sh $(DESTDIR)$(PREFIX)/libexec/configure/scripts/doc/gtkdoc.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/libexec/configure/scripts/doc
	$(INSTALL) -m 0644 doc/manual.css.xml $(DESTDIR)$(PREFIX)/libexec/configure/scripts/doc/manual.css.xml
	$(MKDIR) $(DESTDIR)$(PREFIX)/libexec/configure/scripts/doc
	$(INSTALL) -m 0755 doc/markdown.sh $(DESTDIR)$(PREFIX)/libexec/configure/scripts/doc/markdown.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/libexec/configure/scripts/po
	$(INSTALL) -m 0755 po/gettext.sh $(DESTDIR)$(PREFIX)/libexec/configure/scripts/po/gettext.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests
	$(INSTALL) -m 0755 tests/clint.sh $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests/clint.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests
	$(INSTALL) -m 0755 tests/coverage.sh $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests/coverage.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests
	$(INSTALL) -m 0755 tests/distcheck.sh $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests/distcheck.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests
	$(INSTALL) -m 0755 tests/fixme.sh $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests/fixme.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests
	$(INSTALL) -m 0755 tests/golint.sh $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests/golint.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests
	$(INSTALL) -m 0755 tests/gotest.sh $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests/gotest.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests
	$(INSTALL) -m 0755 tests/htmllint.sh $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests/htmllint.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests
	$(INSTALL) -m 0755 tests/phplint.sh $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests/phplint.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests
	$(INSTALL) -m 0755 tests/pkgconfig.sh $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests/pkgconfig.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests
	$(INSTALL) -m 0755 tests/pylint.sh $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests/pylint.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests
	$(INSTALL) -m 0755 tests/shlint.sh $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests/shlint.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests
	$(INSTALL) -m 0755 tests/xmllint.sh $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests/xmllint.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tools
	$(INSTALL) -m 0755 tools/appbroker.sh $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tools/appbroker.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tools
	$(INSTALL) -m 0755 tools/platform.sh $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tools/platform.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tools
	$(INSTALL) -m 0755 tools/subst.sh $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tools/subst.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tools
	$(INSTALL) -m 0755 tools/template.sh $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tools/template.sh

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/libexec/configure/scripts/data/pkgconfig.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/libexec/configure/scripts/doc/docbook.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/libexec/configure/scripts/doc/gtkdoc.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/libexec/configure/scripts/doc/manual.css.xml
	$(RM) -- $(DESTDIR)$(PREFIX)/libexec/configure/scripts/doc/markdown.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/libexec/configure/scripts/po/gettext.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests/clint.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests/coverage.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests/distcheck.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests/fixme.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests/golint.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests/gotest.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests/htmllint.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests/phplint.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests/pkgconfig.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests/pylint.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests/shlint.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tests/xmllint.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tools/appbroker.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tools/platform.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tools/subst.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/libexec/configure/scripts/tools/template.sh

.PHONY: all clean distclean install uninstall
