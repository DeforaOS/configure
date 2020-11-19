SUBDIRS	= gtkdoc
TARGETS	= $(OBJDIR)clint.log $(OBJDIR)coverage.log $(OBJDIR)fixme.log $(OBJDIR)htmllint.log $(OBJDIR)phplint.log $(OBJDIR)pylint.log $(OBJDIR)shlint.log $(OBJDIR)template-test.md $(OBJDIR)tests.log $(OBJDIR)xmllint.log
OBJDIR	=
PREFIX	= /usr/local
DESTDIR	=
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -m 0755 -p
INSTALL	= install


all: subdirs $(OBJDIR)clint.log $(OBJDIR)htmllint.log $(OBJDIR)phplint.log $(OBJDIR)shlint.log $(OBJDIR)template-test.md $(OBJDIR)tests.log $(OBJDIR)xmllint.log

subdirs:
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		([ -d "$(OBJDIR)$$i" ] || $(MKDIR) -- "$(OBJDIR)$$i") && \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/"; \
		else $(MAKE); fi) || exit; done

$(OBJDIR)clint.log: ../doc/scripts/tests/clint.sh
	../doc/scripts/tests/clint.sh -P "$(PREFIX)" -- "$(OBJDIR)clint.log"

$(OBJDIR)coverage.log: ../doc/scripts/tests/coverage.sh
	../doc/scripts/tests/coverage.sh -P "$(PREFIX)" -- "$(OBJDIR)coverage.log"

$(OBJDIR)fixme.log: ../doc/scripts/tests/fixme.sh
	../doc/scripts/tests/fixme.sh -P "$(PREFIX)" -- "$(OBJDIR)fixme.log"

$(OBJDIR)htmllint.log: ../doc/scripts/tests/htmllint.sh
	../doc/scripts/tests/htmllint.sh -P "$(PREFIX)" -- "$(OBJDIR)htmllint.log"

$(OBJDIR)phplint.log: ../doc/scripts/tests/phplint.sh
	../doc/scripts/tests/phplint.sh -P "$(PREFIX)" -- "$(OBJDIR)phplint.log"

$(OBJDIR)pylint.log: ../doc/scripts/tests/pylint.sh
	../doc/scripts/tests/pylint.sh -P "$(PREFIX)" -- "$(OBJDIR)pylint.log"

$(OBJDIR)shlint.log: ../doc/scripts/data/pkgconfig.sh ../doc/scripts/doc/docbook.sh ../doc/scripts/doc/gtkdoc.sh ../doc/scripts/doc/markdown.sh ../doc/scripts/po/gettext.sh ../doc/scripts/tests/clint.sh ../doc/scripts/tests/coverage.sh ../doc/scripts/tests/fixme.sh ../doc/scripts/tests/htmllint.sh ../doc/scripts/tests/phplint.sh ../doc/scripts/tests/pylint.sh ../doc/scripts/tests/shlint.sh ../doc/scripts/tests/xmllint.sh ../doc/scripts/tools/appbroker.sh ../doc/scripts/tools/platform.sh ../doc/scripts/tools/subst.sh ../doc/scripts/tools/template.sh tests.sh
	../doc/scripts/tests/shlint.sh -P "$(PREFIX)" -- "$(OBJDIR)shlint.log"

$(OBJDIR)template-test.md: template.md.in test.db ../doc/scripts/tools/template.sh ../config.sh
	../doc/scripts/tools/template.sh -P "$(PREFIX)" -O DATABASEDIR=. -- "$(OBJDIR)template-test.md"

$(OBJDIR)tests.log: $(OBJDIR)../tools/configure$(EXEEXT) binary/project.conf binary/Makefile.Darwin binary/Makefile.DeforaOS binary/Makefile.NetBSD binary/Makefile.Windows command/project.conf command/Makefile.NetBSD include/project.conf include/Makefile.NetBSD java/project.conf java/Makefile.NetBSD library/project.conf library/Makefile.Darwin library/Makefile.Linux library/Makefile.NetBSD library/Makefile.Windows libtool/project.conf libtool/Makefile.Darwin libtool/Makefile.Linux libtool/Makefile.NetBSD libtool/Makefile.Windows object/project.conf object/Makefile.NetBSD package/project.conf package/Makefile.NetBSD package/config.ent.NetBSD package/config.h.NetBSD package/config.sh.NetBSD plugin/project.conf plugin/Makefile.Darwin plugin/Makefile.NetBSD script/project.conf script/Makefile.NetBSD verilog/project.conf verilog/Makefile.NetBSD tests.sh
	./tests.sh -P "$(PREFIX)" -- "$(OBJDIR)tests.log"

$(OBJDIR)xmllint.log: ../doc/scripts/tests/xmllint.sh
	../doc/scripts/tests/xmllint.sh -P "$(PREFIX)" -- "$(OBJDIR)xmllint.log"

clean:
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/" clean; \
		else $(MAKE) clean; fi) || exit; done
	../doc/scripts/tests/clint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)clint.log"
	../doc/scripts/tests/coverage.sh -c -P "$(PREFIX)" -- "$(OBJDIR)coverage.log"
	../doc/scripts/tests/fixme.sh -c -P "$(PREFIX)" -- "$(OBJDIR)fixme.log"
	../doc/scripts/tests/htmllint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)htmllint.log"
	../doc/scripts/tests/phplint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)phplint.log"
	../doc/scripts/tests/pylint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)pylint.log"
	../doc/scripts/tests/shlint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)shlint.log"
	../doc/scripts/tools/template.sh -c -P "$(PREFIX)" -O DATABASEDIR=. -- "$(OBJDIR)template-test.md"
	./tests.sh -c -P "$(PREFIX)" -- "$(OBJDIR)tests.log"
	../doc/scripts/tests/xmllint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)xmllint.log"

distclean:
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/" distclean; \
		else $(MAKE) distclean; fi) || exit; done
	../doc/scripts/tests/clint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)clint.log"
	../doc/scripts/tests/coverage.sh -c -P "$(PREFIX)" -- "$(OBJDIR)coverage.log"
	../doc/scripts/tests/fixme.sh -c -P "$(PREFIX)" -- "$(OBJDIR)fixme.log"
	../doc/scripts/tests/htmllint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)htmllint.log"
	../doc/scripts/tests/phplint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)phplint.log"
	../doc/scripts/tests/pylint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)pylint.log"
	../doc/scripts/tests/shlint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)shlint.log"
	../doc/scripts/tools/template.sh -c -P "$(PREFIX)" -O DATABASEDIR=. -- "$(OBJDIR)template-test.md"
	./tests.sh -c -P "$(PREFIX)" -- "$(OBJDIR)tests.log"
	../doc/scripts/tests/xmllint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)xmllint.log"
	$(RM) -- $(TARGETS)

install: all
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/" install; \
		else $(MAKE) install; fi) || exit; done

uninstall:
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/" uninstall; \
		else $(MAKE) uninstall; fi) || exit; done

.PHONY: all subdirs clean distclean install uninstall
