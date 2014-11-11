TARGETS	= $(OBJDIR)tests.log
OBJDIR	=
PREFIX	= /usr/local
DESTDIR	=
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -m 0755 -p
INSTALL	= install


all: $(TARGETS)

$(OBJDIR)tests.log: binary/project.conf binary/Makefile.Darwin binary/Makefile.NetBSD binary/Makefile.Windows library/project.conf library/Makefile.Darwin library/Makefile.NetBSD library/Makefile.Windows plugin/Makefile.Darwin plugin/Makefile.NetBSD tests.sh
	./tests.sh -P "$(PREFIX)" -- "$(OBJDIR)tests.log"

clean:
	$(RM) -- $(tests.log_OBJS)
	./tests.sh -c -P "$(PREFIX)" -- "tests.log"

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)

uninstall:

.PHONY: all clean distclean install uninstall
