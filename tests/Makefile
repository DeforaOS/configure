TARGETS	= $(OBJDIR)tests.log
OBJDIR	=
PREFIX	= /usr/local
DESTDIR	=
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -m 0755 -p
INSTALL	= install


all: $(TARGETS)

$(OBJDIR)tests.log: $(OBJDIR)../src/configure$(EXEEXT) binary/project.conf binary/Makefile.Darwin binary/Makefile.NetBSD binary/Makefile.Windows library/project.conf library/Makefile.Darwin library/Makefile.Linux library/Makefile.NetBSD library/Makefile.Windows libtool/project.conf libtool/Makefile.Darwin libtool/Makefile.Linux libtool/Makefile.NetBSD libtool/Makefile.Windows object/project.conf object/Makefile.NetBSD package/project.conf package/Makefile.NetBSD plugin/Makefile.Darwin plugin/Makefile.NetBSD script/project.conf script/Makefile.NetBSD tests.sh
	./tests.sh -P "$(PREFIX)" -- "$(OBJDIR)tests.log"

clean:
	$(RM) -- $(tests.log_OBJS)
	./tests.sh -c -P "$(PREFIX)" -- "tests.log"

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)

uninstall:

.PHONY: all clean distclean install uninstall
