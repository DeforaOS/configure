TARGETS	= $(OBJDIR)configure $(OBJDIR)makedepend $(OBJDIR)makefile.o
OBJDIR	=
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
SBINDIR	= $(PREFIX)/sbin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= -W `pkg-config --cflags libSystem`
CFLAGS	= -Wall -g -O2 -pedantic
LDFLAGSF= `pkg-config --libs libSystem`
LDFLAGS	= 
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -m 0755 -p
INSTALL	= install


all: $(TARGETS)

configure_OBJS = $(OBJDIR)configure.o $(OBJDIR)settings.o $(OBJDIR)main.o
configure_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
configure_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) $(OBJDIR)makefile.o

$(OBJDIR)configure: $(configure_OBJS) $(OBJDIR)makefile.o
	$(CC) -o $(OBJDIR)configure $(configure_OBJS) $(configure_LDFLAGS)

makedepend_OBJS = $(OBJDIR)makedepend.o
makedepend_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
makedepend_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

$(OBJDIR)makedepend: $(makedepend_OBJS)
	$(CC) -o $(OBJDIR)makedepend $(makedepend_OBJS) $(makedepend_LDFLAGS)

makefile.o_OBJS = $(OBJDIR)makefile.o
makefile.o_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS) -o $(OBJDIR)makefile.o

$(OBJDIR)configure.o: configure.c configure.h makefile.h ../config.h
	$(CC) $(configure_CFLAGS) -o $(OBJDIR)configure.o -c configure.c

$(OBJDIR)settings.o: settings.c settings.h
	$(CC) $(configure_CFLAGS) -o $(OBJDIR)settings.o -c settings.c

$(OBJDIR)main.o: main.c configure.h
	$(CC) $(configure_CFLAGS) -o $(OBJDIR)main.o -c main.c

$(OBJDIR)makedepend.o: makedepend.c
	$(CC) $(makedepend_CFLAGS) -o $(OBJDIR)makedepend.o -c makedepend.c

$(OBJDIR)makefile.o: makefile.c configure.h settings.h ../config.h
	$(CC) $(makefile.o_CFLAGS) -o $(OBJDIR)makefile.o -c makefile.c

clean:
	$(RM) -- $(configure_OBJS) $(makedepend_OBJS) $(makefile.o_OBJS)

distclean: clean
	$(RM) -- $(TARGETS)

install: $(TARGETS)
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 $(OBJDIR)configure $(DESTDIR)$(BINDIR)/configure

uninstall:
	$(RM) -- $(DESTDIR)$(BINDIR)/configure

.PHONY: all clean distclean install uninstall
