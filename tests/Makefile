TARGETS	= $(OBJDIR)clint.log $(OBJDIR)shlint.log $(OBJDIR)tests.log $(OBJDIR)xmllint.log
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

$(OBJDIR)shlint.log: ../doc/scripts/shlint.sh
	../doc/scripts/shlint.sh -P "$(PREFIX)" -- "$(OBJDIR)shlint.log"

$(OBJDIR)tests.log: $(OBJDIR)../src/configure$(EXEEXT) binary/project.conf binary/Makefile.Darwin binary/Makefile.NetBSD binary/Makefile.Windows include/project.conf include/Makefile.NetBSD library/project.conf library/Makefile.Darwin library/Makefile.Linux library/Makefile.NetBSD library/Makefile.Windows libtool/project.conf libtool/Makefile.Darwin libtool/Makefile.Linux libtool/Makefile.NetBSD libtool/Makefile.Windows object/project.conf object/Makefile.NetBSD package/project.conf package/Makefile.NetBSD plugin/Makefile.Darwin plugin/Makefile.NetBSD script/project.conf script/Makefile.NetBSD verilog/project.conf verilog/Makefile.NetBSD tests.sh
	./tests.sh -P "$(PREFIX)" -- "$(OBJDIR)tests.log"

$(OBJDIR)xmllint.log: ../doc/scripts/xmllint.sh
	../doc/scripts/xmllint.sh -P "$(PREFIX)" -- "$(OBJDIR)xmllint.log"

clean:
	$(RM) -- $(clint.log_OBJS) $(shlint.log_OBJS) $(tests.log_OBJS) $(xmllint.log_OBJS)
	../doc/scripts/clint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)clint.log"
	../doc/scripts/shlint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)shlint.log"
	./tests.sh -c -P "$(PREFIX)" -- "$(OBJDIR)tests.log"
	../doc/scripts/xmllint.sh -c -P "$(PREFIX)" -- "$(OBJDIR)xmllint.log"

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)

uninstall:

.PHONY: all clean distclean install uninstall
