OBJDIR	=
PREFIX	= /usr/local
DESTDIR	=
MKDIR	= mkdir -m 0755 -p
INSTALL	= install
RM	= rm -f


all:

clean:

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/configure
	$(INSTALL) -m 0644 configure.conf $(DESTDIR)$(PREFIX)/share/configure/configure.conf

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/share/configure/configure.conf

.PHONY: all clean distclean install uninstall
