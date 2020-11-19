SUBDIRS	= scripts
TARGETS	= $(OBJDIR)configure$(EXEEXT) $(OBJDIR)makedepend$(EXEEXT) $(OBJDIR)makefile.o
OBJDIR	=
PREFIX	= /usr/local
DESTDIR	=
BINDIR	= $(PREFIX)/bin
SBINDIR	= $(PREFIX)/sbin
CC	= cc
CPPFLAGSF=
CPPFLAGS=
CFLAGSF	= `pkg-config --cflags libSystem`
CFLAGS	= -W -Wall -g -O2 -fPIE -D_FORTIFY_SOURCE=2 -fstack-protector
LDFLAGSF= `pkg-config --libs libSystem`
LDFLAGS	= -pie -Wl,-z,relro -Wl,-z,now
EXEEXT	=
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -m 0755 -p
INSTALL	= install


all: subdirs $(TARGETS)

subdirs:
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		([ -d "$(OBJDIR)$$i" ] || $(MKDIR) -- "$(OBJDIR)$$i") && \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/"; \
		else $(MAKE); fi) || exit; done

configure_OBJS = $(OBJDIR)common.o $(OBJDIR)configure.o $(OBJDIR)settings.o $(OBJDIR)main.o
configure_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
configure_LDFLAGS = $(LDFLAGSF) $(LDFLAGS) $(OBJDIR)makefile.o

$(OBJDIR)configure$(EXEEXT): $(configure_OBJS) $(OBJDIR)makefile.o
	$(CC) -o $(OBJDIR)configure$(EXEEXT) $(configure_OBJS) $(configure_LDFLAGS)

makedepend_OBJS = $(OBJDIR)makedepend.o
makedepend_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)
makedepend_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)

$(OBJDIR)makedepend$(EXEEXT): $(makedepend_OBJS)
	$(CC) -o $(OBJDIR)makedepend$(EXEEXT) $(makedepend_OBJS) $(makedepend_LDFLAGS)

makefile.o_OBJS = $(OBJDIR)makefile.o
makefile.o_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF) $(CFLAGS)

$(OBJDIR)common.o: common.c common.h
	$(CC) $(configure_CFLAGS) -o $(OBJDIR)common.o -c common.c

$(OBJDIR)configure.o: configure.c common.h configure.h makefile.h ../config.h
	$(CC) -D PREFIX=\"$(PREFIX)\" $(configure_CFLAGS) -o $(OBJDIR)configure.o -c configure.c

$(OBJDIR)settings.o: settings.c settings.h
	$(CC) $(configure_CFLAGS) -o $(OBJDIR)settings.o -c settings.c

$(OBJDIR)main.o: main.c configure.h
	$(CC) $(configure_CFLAGS) -o $(OBJDIR)main.o -c main.c

$(OBJDIR)makedepend.o: makedepend.c
	$(CC) $(makedepend_CFLAGS) -o $(OBJDIR)makedepend.o -c makedepend.c

$(OBJDIR)makefile.o: makefile.c configure.h settings.h ../config.h
	$(CC) $(makefile.o_CFLAGS) -o $(OBJDIR)makefile.o -c makefile.c

clean:
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/" clean; \
		else $(MAKE) clean; fi) || exit; done
	$(RM) -- $(configure_OBJS) $(makedepend_OBJS) $(makefile.o_OBJS)

distclean:
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/" distclean; \
		else $(MAKE) distclean; fi) || exit; done
	$(RM) -- $(configure_OBJS) $(makedepend_OBJS) $(makefile.o_OBJS)
	$(RM) -- $(TARGETS)

install: all
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/" install; \
		else $(MAKE) install; fi) || exit; done
	$(MKDIR) $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 $(OBJDIR)configure$(EXEEXT) $(DESTDIR)$(BINDIR)/configure$(EXEEXT)

uninstall:
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/" uninstall; \
		else $(MAKE) uninstall; fi) || exit; done
	$(RM) -- $(DESTDIR)$(BINDIR)/configure$(EXEEXT)

.PHONY: all subdirs clean distclean install uninstall
