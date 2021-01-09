PACKAGE	= configure
VERSION	= 0.4.2
VENDOR	= Devel
SUBDIRS	= data doc src tools tests
OBJDIR	=
PREFIX	= /usr/local
DESTDIR	=
MKDIR	= mkdir -m 0755 -p
INSTALL	= install
RM	= rm -f
TARGETS	= tests
RM	= rm -f
LN	= ln -f
TAR	= tar
TGZEXT	= .tar.gz
MKDIR	= mkdir -m 0755 -p
INSTALL	= install


all: subdirs

subdirs:
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		([ -d "$(OBJDIR)$$i" ] || $(MKDIR) -- "$(OBJDIR)$$i") && \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/"; \
		else $(MAKE); fi) || exit; done

tests: all
	cd tests && (if [ -n "$(OBJDIR)" ]; then $(MAKE) OBJDIR="$(OBJDIR)tests/" "$(OBJDIR)tests/clint.log" "$(OBJDIR)tests/coverage.log" "$(OBJDIR)tests/distcheck.log" "$(OBJDIR)tests/fixme.log" "$(OBJDIR)tests/htmllint.log" "$(OBJDIR)tests/phplint.log" "$(OBJDIR)tests/pylint.log" "$(OBJDIR)tests/shlint.log" "$(OBJDIR)tests/tests.log" "$(OBJDIR)tests/xmllint.log"; else $(MAKE) clint.log coverage.log distcheck.log fixme.log htmllint.log phplint.log pylint.log shlint.log tests.log xmllint.log; fi)

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
	@cd $(OBJDIR). && $(TAR) -czvf $(PACKAGE)-$(VERSION)$(TGZEXT) -- \
		$(PACKAGE)-$(VERSION)/data/Makefile \
		$(PACKAGE)-$(VERSION)/data/configure.conf \
		$(PACKAGE)-$(VERSION)/data/project.conf \
		$(PACKAGE)-$(VERSION)/data/platform/Makefile \
		$(PACKAGE)-$(VERSION)/data/platform/Darwin.conf \
		$(PACKAGE)-$(VERSION)/data/platform/DeforaOS.conf \
		$(PACKAGE)-$(VERSION)/data/platform/FreeBSD.conf \
		$(PACKAGE)-$(VERSION)/data/platform/Linux.conf \
		$(PACKAGE)-$(VERSION)/data/platform/NetBSD.conf \
		$(PACKAGE)-$(VERSION)/data/platform/OpenBSD.conf \
		$(PACKAGE)-$(VERSION)/data/platform/Windows.conf \
		$(PACKAGE)-$(VERSION)/data/platform/project.conf \
		$(PACKAGE)-$(VERSION)/doc/Makefile \
		$(PACKAGE)-$(VERSION)/doc/configure.css.xml \
		$(PACKAGE)-$(VERSION)/doc/configure.xml \
		$(PACKAGE)-$(VERSION)/doc/manual.css.xml \
		$(PACKAGE)-$(VERSION)/doc/project.conf.css.xml \
		$(PACKAGE)-$(VERSION)/doc/project.conf.xml \
		$(PACKAGE)-$(VERSION)/doc/project.conf \
		$(PACKAGE)-$(VERSION)/src/common.c \
		$(PACKAGE)-$(VERSION)/src/configure.c \
		$(PACKAGE)-$(VERSION)/src/settings.c \
		$(PACKAGE)-$(VERSION)/src/main.c \
		$(PACKAGE)-$(VERSION)/src/makedepend.c \
		$(PACKAGE)-$(VERSION)/src/makefile.c \
		$(PACKAGE)-$(VERSION)/src/Makefile \
		$(PACKAGE)-$(VERSION)/src/common.h \
		$(PACKAGE)-$(VERSION)/src/configure.h \
		$(PACKAGE)-$(VERSION)/src/makefile.h \
		$(PACKAGE)-$(VERSION)/src/settings.h \
		$(PACKAGE)-$(VERSION)/src/project.conf \
		$(PACKAGE)-$(VERSION)/src/scripts/Makefile \
		$(PACKAGE)-$(VERSION)/src/scripts/config.sh \
		$(PACKAGE)-$(VERSION)/src/scripts/data/pkgconfig.sh \
		$(PACKAGE)-$(VERSION)/src/scripts/doc/docbook.sh \
		$(PACKAGE)-$(VERSION)/src/scripts/doc/gtkdoc.sh \
		$(PACKAGE)-$(VERSION)/src/scripts/doc/manual.css.xml \
		$(PACKAGE)-$(VERSION)/src/scripts/doc/markdown.sh \
		$(PACKAGE)-$(VERSION)/src/scripts/po/gettext.sh \
		$(PACKAGE)-$(VERSION)/src/scripts/tests/clint.sh \
		$(PACKAGE)-$(VERSION)/src/scripts/tests/coverage.sh \
		$(PACKAGE)-$(VERSION)/src/scripts/tests/distcheck.sh \
		$(PACKAGE)-$(VERSION)/src/scripts/tests/fixme.sh \
		$(PACKAGE)-$(VERSION)/src/scripts/tests/htmllint.sh \
		$(PACKAGE)-$(VERSION)/src/scripts/tests/phplint.sh \
		$(PACKAGE)-$(VERSION)/src/scripts/tests/pylint.sh \
		$(PACKAGE)-$(VERSION)/src/scripts/tests/shlint.sh \
		$(PACKAGE)-$(VERSION)/src/scripts/tests/xmllint.sh \
		$(PACKAGE)-$(VERSION)/src/scripts/tools/appbroker.sh \
		$(PACKAGE)-$(VERSION)/src/scripts/tools/platform.sh \
		$(PACKAGE)-$(VERSION)/src/scripts/tools/subst.sh \
		$(PACKAGE)-$(VERSION)/src/scripts/tools/template.sh \
		$(PACKAGE)-$(VERSION)/src/scripts/project.conf \
		$(PACKAGE)-$(VERSION)/tools/configure.c \
		$(PACKAGE)-$(VERSION)/tools/pkg-config.c \
		$(PACKAGE)-$(VERSION)/tools/Makefile \
		$(PACKAGE)-$(VERSION)/tools/configure-update \
		$(PACKAGE)-$(VERSION)/tools/project.conf \
		$(PACKAGE)-$(VERSION)/tests/Makefile \
		$(PACKAGE)-$(VERSION)/tests/binary/project.conf \
		$(PACKAGE)-$(VERSION)/tests/binary/Makefile.Darwin \
		$(PACKAGE)-$(VERSION)/tests/binary/Makefile.DeforaOS \
		$(PACKAGE)-$(VERSION)/tests/binary/Makefile.NetBSD \
		$(PACKAGE)-$(VERSION)/tests/binary/Makefile.Windows \
		$(PACKAGE)-$(VERSION)/tests/command/project.conf \
		$(PACKAGE)-$(VERSION)/tests/command/Makefile.NetBSD \
		$(PACKAGE)-$(VERSION)/tests/config.sh \
		$(PACKAGE)-$(VERSION)/tests/include/project.conf \
		$(PACKAGE)-$(VERSION)/tests/include/Makefile.NetBSD \
		$(PACKAGE)-$(VERSION)/tests/java/project.conf \
		$(PACKAGE)-$(VERSION)/tests/java/Makefile.NetBSD \
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
		$(PACKAGE)-$(VERSION)/tests/mode/project.conf \
		$(PACKAGE)-$(VERSION)/tests/mode/Makefile.NetBSD \
		$(PACKAGE)-$(VERSION)/tests/object/project.conf \
		$(PACKAGE)-$(VERSION)/tests/object/Makefile.NetBSD \
		$(PACKAGE)-$(VERSION)/tests/package/project.conf \
		$(PACKAGE)-$(VERSION)/tests/package/Makefile.NetBSD \
		$(PACKAGE)-$(VERSION)/tests/package/config.ent.NetBSD \
		$(PACKAGE)-$(VERSION)/tests/package/config.h.NetBSD \
		$(PACKAGE)-$(VERSION)/tests/package/config.sh.NetBSD \
		$(PACKAGE)-$(VERSION)/tests/plugin/project.conf \
		$(PACKAGE)-$(VERSION)/tests/plugin/Makefile.Darwin \
		$(PACKAGE)-$(VERSION)/tests/plugin/Makefile.NetBSD \
		$(PACKAGE)-$(VERSION)/tests/script/project.conf \
		$(PACKAGE)-$(VERSION)/tests/script/Makefile.NetBSD \
		$(PACKAGE)-$(VERSION)/tests/template.md.in \
		$(PACKAGE)-$(VERSION)/tests/test.db \
		$(PACKAGE)-$(VERSION)/tests/tests.sh \
		$(PACKAGE)-$(VERSION)/tests/verilog/project.conf \
		$(PACKAGE)-$(VERSION)/tests/verilog/Makefile.NetBSD \
		$(PACKAGE)-$(VERSION)/tests/project.conf \
		$(PACKAGE)-$(VERSION)/tests/gtkdoc/Makefile \
		$(PACKAGE)-$(VERSION)/tests/gtkdoc/project.conf \
		$(PACKAGE)-$(VERSION)/tests/gtkdoc/gtkdoc/Makefile \
		$(PACKAGE)-$(VERSION)/tests/gtkdoc/gtkdoc/example-docs.xml \
		$(PACKAGE)-$(VERSION)/tests/gtkdoc/gtkdoc/example-sections.txt \
		$(PACKAGE)-$(VERSION)/tests/gtkdoc/gtkdoc/xml/gtkdocentities.ent \
		$(PACKAGE)-$(VERSION)/tests/gtkdoc/gtkdoc/project.conf \
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
	$(TAR) -xzvf $(OBJDIR)$(PACKAGE)-$(VERSION)$(TGZEXT)
	$(MKDIR) -- $(PACKAGE)-$(VERSION)/objdir
	$(MKDIR) -- $(PACKAGE)-$(VERSION)/destdir
	cd "$(PACKAGE)-$(VERSION)" && $(MAKE) OBJDIR="$$PWD/objdir/"
	cd "$(PACKAGE)-$(VERSION)" && $(MAKE) OBJDIR="$$PWD/objdir/" DESTDIR="$$PWD/destdir" install
	cd "$(PACKAGE)-$(VERSION)" && $(MAKE) OBJDIR="$$PWD/objdir/" DESTDIR="$$PWD/destdir" uninstall
	cd "$(PACKAGE)-$(VERSION)" && $(MAKE) OBJDIR="$$PWD/objdir/" distclean
	cd "$(PACKAGE)-$(VERSION)" && $(MAKE) dist
	$(RM) -r -- $(PACKAGE)-$(VERSION)

install: all
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
	$(INSTALL) -m 0644 COPYING $(DESTDIR)$(PREFIX)/share/doc/configure/COPYING
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
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/COPYING
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/INSTALL.md
	$(RM) -- $(DESTDIR)$(PREFIX)/share/doc/configure/README.md

.PHONY: all subdirs clean distclean dist distcheck install uninstall tests
