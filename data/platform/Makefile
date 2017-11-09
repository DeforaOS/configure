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
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/configure/platform
	$(INSTALL) -m 0644 Darwin.conf $(DESTDIR)$(PREFIX)/share/configure/platform/Darwin.conf
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/configure/platform
	$(INSTALL) -m 0644 FreeBSD.conf $(DESTDIR)$(PREFIX)/share/configure/platform/FreeBSD.conf
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/configure/platform
	$(INSTALL) -m 0644 Linux.conf $(DESTDIR)$(PREFIX)/share/configure/platform/Linux.conf
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/configure/platform
	$(INSTALL) -m 0644 NetBSD.conf $(DESTDIR)$(PREFIX)/share/configure/platform/NetBSD.conf
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/configure/platform
	$(INSTALL) -m 0644 OpenBSD.conf $(DESTDIR)$(PREFIX)/share/configure/platform/OpenBSD.conf
	$(MKDIR) $(DESTDIR)$(PREFIX)/share/configure/platform
	$(INSTALL) -m 0644 Windows.conf $(DESTDIR)$(PREFIX)/share/configure/platform/Windows.conf

uninstall:
	$(RM) -- $(DESTDIR)$(PREFIX)/share/configure/platform/Darwin.conf
	$(RM) -- $(DESTDIR)$(PREFIX)/share/configure/platform/FreeBSD.conf
	$(RM) -- $(DESTDIR)$(PREFIX)/share/configure/platform/Linux.conf
	$(RM) -- $(DESTDIR)$(PREFIX)/share/configure/platform/NetBSD.conf
	$(RM) -- $(DESTDIR)$(PREFIX)/share/configure/platform/OpenBSD.conf
	$(RM) -- $(DESTDIR)$(PREFIX)/share/configure/platform/Windows.conf

.PHONY: all clean distclean install uninstall
