TARGETS	= $(OBJDIR)configure$(EXEEXT) $(OBJDIR)pkg-config$(EXEEXT)
OBJDIR	=
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
SBINDIR	= $(PREFIX)/sbin
CC	= cc
CPPFLAGSF= -D PREFIX=\"$(PREFIX)\"
CPPFLAGS=
CFLAGSF	=
CFLAGS	= -W -Wall -g -O2 -fPIE -D_FORTIFY_SOURCE=2 -fstack-protector
LDFLAGS	= -pie -Wl,-z,relro -Wl,-z,now
EXEEXT	=
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -m 0755 -p
INSTALL	= install


all: $(TARGETS)

configure_OBJS = $(OBJDIR)configure.o
configure_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) `pkg-config --cflags libSystem`
configure_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) `pkg-config --libs libSystem`

$(OBJDIR)configure$(EXEEXT): $(configure_OBJS)
	$(CC) -o $(OBJDIR)configure$(EXEEXT) $(configure_OBJS) $(configure_LDFLAGS)

pkg-config_OBJS = $(OBJDIR)pkg-config.o
pkg-config_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
pkg-config_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

$(OBJDIR)pkg-config$(EXEEXT): $(pkg-config_OBJS)
	$(CC) -o $(OBJDIR)pkg-config$(EXEEXT) $(pkg-config_OBJS) $(pkg-config_LDFLAGS)

$(OBJDIR)configure.o: configure.c ../src/configure.c ../src/configure.h ../src/main.c ../src/makefile.c ../src/makefile.h ../src/settings.c ../src/settings.h
	$(CC) $(configure_CFLAGS) -o $(OBJDIR)configure.o -c configure.c

$(OBJDIR)pkg-config.o: pkg-config.c ../config.h
	$(CC) $(pkg-config_CFLAGS) -o $(OBJDIR)pkg-config.o -c pkg-config.c

clean:
	$(RM) -- $(configure_OBJS) $(pkg-config_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)

uninstall:

.PHONY: all clean distclean install uninstall
