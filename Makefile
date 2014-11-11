PACKAGE	= configure
VERSION	= 0.1.0
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


all: subdirs

subdirs:
	@for i in $(SUBDIRS); do (cd "$$i" && $(MAKE)) || exit; done

clean:
	@for i in $(SUBDIRS); do (cd "$$i" && $(MAKE) clean) || exit; done

distclean:
	@for i in $(SUBDIRS); do (cd "$$i" && $(MAKE) distclean) || exit; done

dist:
	$(RM) -r -- $(PACKAGE)-$(VERSION)
	$(LN) -s -- . $(PACKAGE)-$(VERSION)
	@$(TAR) -czvf $(PACKAGE)-$(VERSION).tar.gz -- \
		$(PACKAGE)-$(VERSION)/doc/Makefile \
		$(PACKAGE)-$(VERSION)/doc/docbook.sh \
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
		$(PACKAGE)-$(VERSION)/tests/library/project.conf \
		$(PACKAGE)-$(VERSION)/tests/library/Makefile.NetBSD \
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
		$(PACKAGE)-$(VERSION)/INSTALL \
		$(PACKAGE)-$(VERSION)/README \
		$(PACKAGE)-$(VERSION)/project.conf
	$(RM) -- $(PACKAGE)-$(VERSION)

distcheck: dist
	$(TAR) -xzvf $(PACKAGE)-$(VERSION).tar.gz
	$(MKDIR) -- $(PACKAGE)-$(VERSION)/objdir
	$(MKDIR) -- $(PACKAGE)-$(VERSION)/destdir
	(cd "$(PACKAGE)-$(VERSION)" && $(MAKE) OBJDIR="$$PWD/objdir/")
	(cd "$(PACKAGE)-$(VERSION)" && $(MAKE) OBJDIR="$$PWD/objdir/" DESTDIR="$$PWD/destdir" install)
	(cd "$(PACKAGE)-$(VERSION)" && $(MAKE) OBJDIR="$$PWD/objdir/" DESTDIR="$$PWD/destdir" uninstall)
	(cd "$(PACKAGE)-$(VERSION)" && $(MAKE) OBJDIR="$$PWD/objdir/" distclean)
	(cd "$(PACKAGE)-$(VERSION)" && $(MAKE) dist)
	$(RM) -r -- $(PACKAGE)-$(VERSION)

install:
	@for i in $(SUBDIRS); do (cd "$$i" && $(MAKE) install) || exit; done
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure
	$(INSTALL) -m 0644 AUTHORS $(DESTDIR)$(PREFIX)/share/doc/configure/AUTHORS
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure
	$(INSTALL) -m 0644 BUGS $(DESTDIR)$(PREFIX)/share/doc/configure/BUGS
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure
	$(INSTALL) -m 0644 CHANGES $(DESTDIR)$(PREFIX)/share/doc/configure/CHANGES
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure
	$(INSTALL) -m 0644 INSTALL $(DESTDIR)$(PREFIX)/share/doc/configure/INSTALL
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/doc/configure
	$(INSTALL) -m 0644 README $(DESTDIR)$(PREFIX)/share/doc/configure/README

uninstall:
	@for i in $(SUBDIRS); do (cd "$$i" && $(MAKE) uninstall) || exit; done
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/AUTHORS
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/BUGS
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/CHANGES
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/INSTALL
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/README

.PHONY: all subdirs clean distclean dist distcheck install uninstall
