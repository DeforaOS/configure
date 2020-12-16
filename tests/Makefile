SUBDIRS	= gtkdoc
TARGETS	= $(OBJDIR)clint.log $(OBJDIR)coverage.log $(OBJDIR)distcheck.log $(OBJDIR)fixme.log $(OBJDIR)htmllint.log $(OBJDIR)phplint.log $(OBJDIR)pylint.log $(OBJDIR)shlint.log $(OBJDIR)template-test.md $(OBJDIR)tests.log $(OBJDIR)xmllint.log
OBJDIR	=
PREFIX	= /usr/local
DESTDIR	=
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -m 0755 -p
INSTALL	= install


all: subdirs $(OBJDIR)template-test.md

subdirs:
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		([ -d "$(OBJDIR)$$i" ] || $(MKDIR) -- "$(OBJDIR)$$i") && \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/"; \
		else $(MAKE); fi) || exit; done

$(OBJDIR)clint.log: ../src/scripts/tests/clint.sh
	../src/scripts/tests/clint.sh -P "$(PREFIX)" -- "$(OBJDIR)clint.log"

$(OBJDIR)coverage.log: ../src/scripts/tests/coverage.sh
	../src/scripts/tests/coverage.sh -P "$(PREFIX)" -- "$(OBJDIR)coverage.log"

$(OBJDIR)distcheck.log: ../src/scripts/tests/distcheck.sh
	../src/scripts/tests/distcheck.sh -P "$(PREFIX)" -- "$(OBJDIR)distcheck.log"

$(OBJDIR)fixme.log: ../src/scripts/tests/fixme.sh
	../src/scripts/tests/fixme.sh -P "$(PREFIX)" -- "$(OBJDIR)fixme.log"

$(OBJDIR)htmllint.log: ../src/scripts/tests/htmllint.sh
	../src/scripts/tests/htmllint.sh -P "$(PREFIX)" -- "$(OBJDIR)htmllint.log"

$(OBJDIR)phplint.log: ../src/scripts/tests/phplint.sh
	../src/scripts/tests/phplint.sh -P "$(PREFIX)" -- "$(OBJDIR)phplint.log"

$(OBJDIR)pylint.log: ../src/scripts/tests/pylint.sh
	../src/scripts/tests/pylint.sh -P "$(PREFIX)" -- "$(OBJDIR)pylint.log"

$(OBJDIR)shlint.log: ../src/scripts/data/pkgconfig.sh ../src/scripts/doc/docbook.sh ../src/scripts/doc/gtkdoc.sh ../src/scripts/doc/markdown.sh ../src/scripts/po/gettext.sh ../src/scripts/tests/clint.sh ../src/scripts/tests/coverage.sh ../src/scripts/tests/fixme.sh ../src/scripts/tests/htmllint.sh ../src/scripts/tests/phplint.sh ../src/scripts/tests/pylint.sh ../src/scripts/tests/shlint.sh ../src/scripts/tests/xmllint.sh ../src/scripts/tools/appbroker.sh ../src/scripts/tools/platform.sh ../src/scripts/tools/subst.sh ../src/scripts/tools/template.sh tests.sh
	../src/scripts/tests/shlint.sh -P "$(PREFIX)" -- "$(OBJDIR)shlint.log"

$(OBJDIR)template-test.md: template.md.in test.db ../src/scripts/tools/template.sh ../config.sh
	../src/scripts/tools/template.sh -P "$(PREFIX)" -O DATABASEDIR=. -- "$(OBJDIR)template-test.md"

$(OBJDIR)tests.log: $(OBJDIR)../tools/configure$(EXEEXT) binary/project.conf binary/Makefile.Darwin binary/Makefile.DeforaOS binary/Makefile.NetBSD binary/Makefile.Windows command/project.conf command/Makefile.NetBSD include/project.conf include/Makefile.NetBSD java/project.conf java/Makefile.NetBSD library/project.conf library/Makefile.Darwin library/Makefile.Linux library/Makefile.NetBSD library/Makefile.Windows libtool/project.conf libtool/Makefile.Darwin libtool/Makefile.Linux libtool/Makefile.NetBSD libtool/Makefile.Windows object/project.conf object/Makefile.NetBSD package/project.conf package/Makefile.NetBSD package/config.ent.NetBSD package/config.h.NetBSD package/config.sh.NetBSD plugin/project.conf plugin/Makefile.Darwin plugin/Makefile.NetBSD script/project.conf script/Makefile.NetBSD verilog/project.conf verilog/Makefile.NetBSD tests.sh
	./tests.sh -P "$(PREFIX)" -- "$(OBJDIR)tests.log"

$(OBJDIR)xmllint.log: ../src/scripts/tests/xmllint.sh
	../src/scripts/tests/xmllint.sh -P "$(PREFIX)" -- "$(OBJDIR)xmllint.log"

clean:
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/" clean; \
		else $(MAKE) clean; fi) || exit; done
	../src/scripts/tests/clint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)clint.log"
	../src/scripts/tests/coverage.sh -c -P "$(PREFIX)" -- "$(OBJDIR)coverage.log"
	../src/scripts/tests/distcheck.sh -c -P "$(PREFIX)" -- "$(OBJDIR)distcheck.log"
	../src/scripts/tests/fixme.sh -c -P "$(PREFIX)" -- "$(OBJDIR)fixme.log"
	../src/scripts/tests/htmllint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)htmllint.log"
	../src/scripts/tests/phplint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)phplint.log"
	../src/scripts/tests/pylint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)pylint.log"
	../src/scripts/tests/shlint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)shlint.log"
	../src/scripts/tools/template.sh -c -P "$(PREFIX)" -O DATABASEDIR=. -- "$(OBJDIR)template-test.md"
	./tests.sh -c -P "$(PREFIX)" -- "$(OBJDIR)tests.log"
	../src/scripts/tests/xmllint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)xmllint.log"

distclean:
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/" distclean; \
		else $(MAKE) distclean; fi) || exit; done
	../src/scripts/tests/clint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)clint.log"
	../src/scripts/tests/coverage.sh -c -P "$(PREFIX)" -- "$(OBJDIR)coverage.log"
	../src/scripts/tests/distcheck.sh -c -P "$(PREFIX)" -- "$(OBJDIR)distcheck.log"
	../src/scripts/tests/fixme.sh -c -P "$(PREFIX)" -- "$(OBJDIR)fixme.log"
	../src/scripts/tests/htmllint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)htmllint.log"
	../src/scripts/tests/phplint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)phplint.log"
	../src/scripts/tests/pylint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)pylint.log"
	../src/scripts/tests/shlint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)shlint.log"
	../src/scripts/tools/template.sh -c -P "$(PREFIX)" -O DATABASEDIR=. -- "$(OBJDIR)template-test.md"
	./tests.sh -c -P "$(PREFIX)" -- "$(OBJDIR)tests.log"
	../src/scripts/tests/xmllint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)xmllint.log"
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
