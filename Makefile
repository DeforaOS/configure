PACKAGE	= configure
VERSION	= 0.2.3
SUBDIRS	= doc src tests tools
OBJDIR	=
PREFIX	= /usr/local
DESTDIR	=
MKDIR	= mkdir -m 0755 -p
INSTALL	= install
RM	= rm -f
RM	= rm -f
LN	= ln -f
TAR	= tar
MKDIR	= mkdir -m 0755 -p


all: subdirs

subdirs:
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		([ -d "$(OBJDIR)$$i" ] || $(MKDIR) -- "$(OBJDIR)$$i") && \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/"; \
		else $(MAKE); fi) || exit; done

clean:
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/" clean; \
		else $(MAKE) clean; fi) || exit; done

distclean:
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/" distclean; \
		else $(MAKE) distclean; fi) || exit; done

dist:
	$(RM) -r -- $(OBJDIR)$(PACKAGE)-$(VERSION)
	$(LN) -s -- "$$PWD" $(OBJDIR)$(PACKAGE)-$(VERSION)
	@cd $(OBJDIR). && $(TAR) -czvf $(PACKAGE)-$(VERSION).tar.gz -- \
		$(PACKAGE)-$(VERSION)/doc/Makefile \
		$(PACKAGE)-$(VERSION)/doc/configure.css.xml \
		$(PACKAGE)-$(VERSION)/doc/configure.xml \
		$(PACKAGE)-$(VERSION)/doc/manual.css.xml \
		$(PACKAGE)-$(VERSION)/doc/project.conf.css.xml \
		$(PACKAGE)-$(VERSION)/doc/project.conf.xml \
		$(PACKAGE)-$(VERSION)/doc/project.conf \
		$(PACKAGE)-$(VERSION)/doc/scripts/Makefile \
		$(PACKAGE)-$(VERSION)/doc/scripts/appbroker.sh \
		$(PACKAGE)-$(VERSION)/doc/scripts/docbook.sh \
		$(PACKAGE)-$(VERSION)/doc/scripts/gettext.sh \
		$(PACKAGE)-$(VERSION)/doc/scripts/gtkdoc.sh \
		$(PACKAGE)-$(VERSION)/doc/scripts/manual.css.xml \
		$(PACKAGE)-$(VERSION)/doc/scripts/phplint.sh \
		$(PACKAGE)-$(VERSION)/doc/scripts/pkgconfig.sh \
		$(PACKAGE)-$(VERSION)/doc/scripts/platform.sh \
		$(PACKAGE)-$(VERSION)/doc/scripts/pylint.sh \
		$(PACKAGE)-$(VERSION)/doc/scripts/shlint.sh \
		$(PACKAGE)-$(VERSION)/doc/scripts/subst.sh \
		$(PACKAGE)-$(VERSION)/doc/scripts/xmllint.sh \
		$(PACKAGE)-$(VERSION)/doc/scripts/project.conf \
		$(PACKAGE)-$(VERSION)/src/configure.c \
		$(PACKAGE)-$(VERSION)/src/settings.c \
		$(PACKAGE)-$(VERSION)/src/main.c \
		$(PACKAGE)-$(VERSION)/src/makedepend.c \
		$(PACKAGE)-$(VERSION)/src/makefile.c \
		$(PACKAGE)-$(VERSION)/src/Makefile \
		$(PACKAGE)-$(VERSION)/src/configure.h \
		$(PACKAGE)-$(VERSION)/src/makefile.h \
		$(PACKAGE)-$(VERSION)/src/settings.h \
		$(PACKAGE)-$(VERSION)/src/project.conf \
		$(PACKAGE)-$(VERSION)/tests/Makefile \
		$(PACKAGE)-$(VERSION)/tests/tests.sh \
		$(PACKAGE)-$(VERSION)/tests/binary/project.conf \
		$(PACKAGE)-$(VERSION)/tests/binary/Makefile.Darwin \
		$(PACKAGE)-$(VERSION)/tests/binary/Makefile.NetBSD \
		$(PACKAGE)-$(VERSION)/tests/binary/Makefile.Windows \
		$(PACKAGE)-$(VERSION)/tests/include/project.conf \
		$(PACKAGE)-$(VERSION)/tests/include/Makefile.NetBSD \
		$(PACKAGE)-$(VERSION)/tests/library/project.conf \
		$(PACKAGE)-$(VERSION)/tests/library/Makefile.Darwin \
		$(PACKAGE)-$(VERSION)/tests/library/Makefile.Linux \
		$(PACKAGE)-$(VERSION)/tests/library/Makefile.NetBSD \
		$(PACKAGE)-$(VERSION)/tests/library/Makefile.Windows \
		$(PACKAGE)-$(VERSION)/tests/libtool/project.conf \
		$(PACKAGE)-$(VERSION)/tests/libtool/Makefile.Darwin \
		$(PACKAGE)-$(VERSION)/tests/libtool/Makefile.Linux \
		$(PACKAGE)-$(VERSION)/tests/libtool/Makefile.NetBSD \
		$(PACKAGE)-$(VERSION)/tests/libtool/Makefile.Windows \
		$(PACKAGE)-$(VERSION)/tests/object/project.conf \
		$(PACKAGE)-$(VERSION)/tests/object/Makefile.NetBSD \
		$(PACKAGE)-$(VERSION)/tests/package/project.conf \
		$(PACKAGE)-$(VERSION)/tests/package/Makefile.NetBSD \
		$(PACKAGE)-$(VERSION)/tests/plugin/project.conf \
		$(PACKAGE)-$(VERSION)/tests/plugin/Makefile.Darwin \
		$(PACKAGE)-$(VERSION)/tests/plugin/Makefile.NetBSD \
		$(PACKAGE)-$(VERSION)/tests/script/project.conf \
		$(PACKAGE)-$(VERSION)/tests/script/Makefile.NetBSD \
		$(PACKAGE)-$(VERSION)/tests/project.conf \
		$(PACKAGE)-$(VERSION)/tools/pkg-config.c \
		$(PACKAGE)-$(VERSION)/tools/Makefile \
		$(PACKAGE)-$(VERSION)/tools/project.conf \
		$(PACKAGE)-$(VERSION)/Makefile \
		$(PACKAGE)-$(VERSION)/AUTHORS \
		$(PACKAGE)-$(VERSION)/BUGS \
		$(PACKAGE)-$(VERSION)/CHANGES \
		$(PACKAGE)-$(VERSION)/COPYING \
		$(PACKAGE)-$(VERSION)/config.h \
		$(PACKAGE)-$(VERSION)/config.sh \
		$(PACKAGE)-$(VERSION)/INSTALL.md \
		$(PACKAGE)-$(VERSION)/README.md \
		$(PACKAGE)-$(VERSION)/project.conf
	$(RM) -- $(OBJDIR)$(PACKAGE)-$(VERSION)

distcheck: dist
	$(TAR) -xzvf $(OBJDIR)$(PACKAGE)-$(VERSION).tar.gz
	$(MKDIR) -- $(PACKAGE)-$(VERSION)/objdir
	$(MKDIR) -- $(PACKAGE)-$(VERSION)/destdir
	cd "$(PACKAGE)-$(VERSION)" && $(MAKE) OBJDIR="$$PWD/objdir/"
	cd "$(PACKAGE)-$(VERSION)" && $(MAKE) OBJDIR="$$PWD/objdir/" DESTDIR="$$PWD/destdir" install
	cd "$(PACKAGE)-$(VERSION)" && $(MAKE) OBJDIR="$$PWD/objdir/" DESTDIR="$$PWD/destdir" uninstall
	cd "$(PACKAGE)-$(VERSION)" && $(MAKE) OBJDIR="$$PWD/objdir/" distclean
	cd "$(PACKAGE)-$(VERSION)" && $(MAKE) dist
	$(RM) -r -- $(PACKAGE)-$(VERSION)

install:
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/" install; \
		else $(MAKE) install; fi) || exit; done
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure
	$(INSTALL) -m 0644 AUTHORS $(DESTDIR)$(PREFIX)/share/doc/configure/AUTHORS
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure
	$(INSTALL) -m 0644 BUGS $(DESTDIR)$(PREFIX)/share/doc/configure/BUGS
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure
	$(INSTALL) -m 0644 CHANGES $(DESTDIR)$(PREFIX)/share/doc/configure/CHANGES
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure
	$(INSTALL) -m 0644 INSTALL.md $(DESTDIR)$(PREFIX)/share/doc/configure/INSTALL.md
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure
	$(INSTALL) -m 0644 README.md $(DESTDIR)$(PREFIX)/share/doc/configure/README.md

uninstall:
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/" uninstall; \
		else $(MAKE) uninstall; fi) || exit; done
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/AUTHORS
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/BUGS
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/CHANGES
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/INSTALL.md
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/README.md

.PHONY: all subdirs clean distclean dist distcheck install uninstall
