TARGETS	= tests.log
PREFIX	= /usr/local
DESTDIR	=
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -m 0755 -p
INSTALL	= install


all: $(TARGETS)

tests.log: ../src/configure library/project.conf library/Makefile.NetBSD tests.sh
	./tests.sh -P "$(PREFIX)" -- "tests.log"

clean:
	$(RM) -- $(tests.log_OBJS)
	./tests.sh -c -P "$(PREFIX)" -- "tests.log"

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)

uninstall:

.PHONY: all clean distclean install uninstall
