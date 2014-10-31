TARGETS	= $(OBJDIR)pkg-config
OBJDIR	=
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
SBINDIR	= $(PREFIX)/sbin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W
CFLAGS	= -Wall -g -O2 -pedantic
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -m 0755 -p
INSTALL	= install


all: $(TARGETS)

pkg-config_OBJS = $(OBJDIR)pkg-config.o
pkg-config_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
pkg-config_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

$(OBJDIR)pkg-config: $(pkg-config_OBJS)
	$(CC) -o $(OBJDIR)pkg-config $(pkg-config_OBJS) $(pkg-config_LDFLAGS)

$(OBJDIR)pkg-config.o: pkg-config.c ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(pkg-config_CFLAGS) -o $(OBJDIR)pkg-config.o -c pkg-config.c

clean:
	$(RM) -- $(pkg-config_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)

uninstall:

.PHONY: all clean distclean install uninstall
