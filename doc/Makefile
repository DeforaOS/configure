SUBDIRS	= scripts
TARGETS	= $(OBJDIR)configure.1 $(OBJDIR)configure.html $(OBJDIR)project.conf.5 $(OBJDIR)project.conf.html
OBJDIR	=
PREFIX	= /usr/local
DESTDIR	=
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

$(OBJDIR)configure.1: scripts/doc/docbook.sh configure.xml
	./scripts/doc/docbook.sh -P "$(PREFIX)" -- "$(OBJDIR)configure.1"

$(OBJDIR)configure.html: scripts/doc/docbook.sh configure.css.xml configure.xml manual.css.xml
	./scripts/doc/docbook.sh -P "$(PREFIX)" -- "$(OBJDIR)configure.html"

$(OBJDIR)project.conf.5: scripts/doc/docbook.sh project.conf.xml
	./scripts/doc/docbook.sh -P "$(PREFIX)" -- "$(OBJDIR)project.conf.5"

$(OBJDIR)project.conf.html: scripts/doc/docbook.sh project.conf.css.xml project.conf.xml manual.css.xml
	./scripts/doc/docbook.sh -P "$(PREFIX)" -- "$(OBJDIR)project.conf.html"

clean:
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/" clean; \
		else $(MAKE) clean; fi) || exit; done
	./scripts/doc/docbook.sh -c -P "$(PREFIX)" -- "$(OBJDIR)configure.1"
	./scripts/doc/docbook.sh -c -P "$(PREFIX)" -- "$(OBJDIR)configure.html"
	./scripts/doc/docbook.sh -c -P "$(PREFIX)" -- "$(OBJDIR)project.conf.5"
	./scripts/doc/docbook.sh -c -P "$(PREFIX)" -- "$(OBJDIR)project.conf.html"

distclean:
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/" distclean; \
		else $(MAKE) distclean; fi) || exit; done
	./scripts/doc/docbook.sh -c -P "$(PREFIX)" -- "$(OBJDIR)configure.1"
	./scripts/doc/docbook.sh -c -P "$(PREFIX)" -- "$(OBJDIR)configure.html"
	./scripts/doc/docbook.sh -c -P "$(PREFIX)" -- "$(OBJDIR)project.conf.5"
	./scripts/doc/docbook.sh -c -P "$(PREFIX)" -- "$(OBJDIR)project.conf.html"
	$(RM) -- $(TARGETS)

install: all
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/" install; \
		else $(MAKE) install; fi) || exit; done
	./scripts/doc/docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "$(OBJDIR)configure.1"
	./scripts/doc/docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "$(OBJDIR)configure.html"
	./scripts/doc/docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "$(OBJDIR)project.conf.5"
	./scripts/doc/docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "$(OBJDIR)project.conf.html"

uninstall:
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/" uninstall; \
		else $(MAKE) uninstall; fi) || exit; done
	./scripts/doc/docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "configure.1"
	./scripts/doc/docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "configure.html"
	./scripts/doc/docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "project.conf.5"
	./scripts/doc/docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "project.conf.html"

.PHONY: all subdirs clean distclean install uninstall
