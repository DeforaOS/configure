OBJDIR	=
PREFIX	= /usr/local
DESTDIR	=
MKDIR	= mkdir -m 0755 -p
INSTALL	= install
RM	= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts
	$(INSTALL) -m 0644 appbroker.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/appbroker.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts
	$(INSTALL) -m 0644 clint.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/clint.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts
	$(INSTALL) -m 0644 docbook.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/docbook.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts
	$(INSTALL) -m 0644 gettext.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/gettext.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts
	$(INSTALL) -m 0644 gtkdoc.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/gtkdoc.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts
	$(INSTALL) -m 0644 manual.css.xml $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/manual.css.xml
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts
	$(INSTALL) -m 0644 phplint.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/phplint.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts
	$(INSTALL) -m 0644 pkgconfig.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/pkgconfig.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts
	$(INSTALL) -m 0644 platform.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/platform.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts
	$(INSTALL) -m 0644 pylint.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/pylint.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts
	$(INSTALL) -m 0644 shlint.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/shlint.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts
	$(INSTALL) -m 0644 subst.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/subst.sh
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure/scripts
	$(INSTALL) -m 0644 xmllint.sh $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/xmllint.sh

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/appbroker.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/clint.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/docbook.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/gettext.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/gtkdoc.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/manual.css.xml
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/phplint.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/pkgconfig.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/platform.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/pylint.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/shlint.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/subst.sh
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/scripts/xmllint.sh

.PHONY: all clean distclean install uninstall
