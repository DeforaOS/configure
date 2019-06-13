TARGETS	= $(OBJDIR)clint.log $(OBJDIR)fixme.log $(OBJDIR)htmllint.log $(OBJDIR)phplint.log $(OBJDIR)pylint.log $(OBJDIR)shlint.log $(OBJDIR)tests.log $(OBJDIR)xmllint.log
OBJDIR	=
PREFIX	= /usr/local
DESTDIR	=
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -m 0755 -p
INSTALL	= install


all: $(TARGETS)

$(OBJDIR)clint.log: ../doc/scripts/clint.sh
	../doc/scripts/clint.sh -P "$(PREFIX)" -- "$(OBJDIR)clint.log"

$(OBJDIR)coverage.log: ../doc/scripts/coverage.sh
	../doc/scripts/coverage.sh -P "$(PREFIX)" -- "$(OBJDIR)coverage.log"

$(OBJDIR)fixme.log: ../doc/scripts/fixme.sh
	../doc/scripts/fixme.sh -P "$(PREFIX)" -- "$(OBJDIR)fixme.log"

$(OBJDIR)htmllint.log: ../doc/scripts/htmllint.sh
	../doc/scripts/htmllint.sh -P "$(PREFIX)" -- "$(OBJDIR)htmllint.log"

$(OBJDIR)phplint.log: ../doc/scripts/phplint.sh
	../doc/scripts/phplint.sh -P "$(PREFIX)" -- "$(OBJDIR)phplint.log"

$(OBJDIR)pylint.log: ../doc/scripts/pylint.sh
	../doc/scripts/pylint.sh -P "$(PREFIX)" -- "$(OBJDIR)pylint.log"

$(OBJDIR)shlint.log: ../doc/scripts/appbroker.sh ../doc/scripts/clint.sh ../doc/scripts/coverage.sh ../doc/scripts/docbook.sh ../doc/scripts/fixme.sh ../doc/scripts/gettext.sh ../doc/scripts/gtkdoc.sh ../doc/scripts/htmllint.sh ../doc/scripts/markdown.sh ../doc/scripts/phplint.sh ../doc/scripts/pkgconfig.sh ../doc/scripts/platform.sh ../doc/scripts/pylint.sh ../doc/scripts/shlint.sh ../doc/scripts/subst.sh ../doc/scripts/template.sh ../doc/scripts/xmllint.sh tests.sh
	../doc/scripts/shlint.sh -P "$(PREFIX)" -- "$(OBJDIR)shlint.log"

$(OBJDIR)tests.log: $(OBJDIR)../tools/configure$(EXEEXT) binary/project.conf binary/Makefile.Darwin binary/Makefile.DeforaOS binary/Makefile.NetBSD binary/Makefile.Windows command/project.conf command/Makefile.NetBSD include/project.conf include/Makefile.NetBSD java/project.conf java/Makefile.NetBSD library/project.conf library/Makefile.Darwin library/Makefile.Linux library/Makefile.NetBSD library/Makefile.Windows libtool/project.conf libtool/Makefile.Darwin libtool/Makefile.Linux libtool/Makefile.NetBSD libtool/Makefile.Windows object/project.conf object/Makefile.NetBSD package/project.conf package/Makefile.NetBSD package/config.h.NetBSD package/config.sh.NetBSD plugin/project.conf plugin/Makefile.Darwin plugin/Makefile.NetBSD script/project.conf script/Makefile.NetBSD verilog/project.conf verilog/Makefile.NetBSD tests.sh
	./tests.sh -P "$(PREFIX)" -- "$(OBJDIR)tests.log"

$(OBJDIR)xmllint.log: ../doc/scripts/xmllint.sh
	../doc/scripts/xmllint.sh -P "$(PREFIX)" -- "$(OBJDIR)xmllint.log"

clean:
	../doc/scripts/clint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)clint.log"
	../doc/scripts/coverage.sh -c -P "$(PREFIX)" -- "$(OBJDIR)coverage.log"
	../doc/scripts/fixme.sh -c -P "$(PREFIX)" -- "$(OBJDIR)fixme.log"
	../doc/scripts/htmllint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)htmllint.log"
	../doc/scripts/phplint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)phplint.log"
	../doc/scripts/pylint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)pylint.log"
	../doc/scripts/shlint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)shlint.log"
	./tests.sh -c -P "$(PREFIX)" -- "$(OBJDIR)tests.log"
	../doc/scripts/xmllint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)xmllint.log"

distclean: clean
	$(RM) -- $(TARGETS)

install: all

uninstall:

.PHONY: all clean distclean install uninstall
