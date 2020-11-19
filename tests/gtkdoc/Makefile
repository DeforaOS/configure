SUBDIRS	= gtkdoc
TARGETS	= $(OBJDIR)gtkdoc/example.types $(OBJDIR)gtkdoc/html.stamp $(OBJDIR)gtkdoc/tmpl.stamp $(OBJDIR)gtkdoc/xml.stamp
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

$(OBJDIR)gtkdoc/example.types: ../../src/scripts/doc/gtkdoc.sh ../config.sh
	../../src/scripts/doc/gtkdoc.sh -P "$(PREFIX)" -- "$(OBJDIR)gtkdoc/example.types"

$(OBJDIR)gtkdoc/html.stamp: ../../src/scripts/doc/gtkdoc.sh ../config.sh gtkdoc/example-docs.xml gtkdoc/xml/gtkdocentities.ent $(OBJDIR)gtkdoc/xml.stamp $(OBJDIR)gtkdoc/tmpl.stamp
	../../src/scripts/doc/gtkdoc.sh -P "$(PREFIX)" -- "$(OBJDIR)gtkdoc/html.stamp"

$(OBJDIR)gtkdoc/tmpl.stamp: ../../src/scripts/doc/gtkdoc.sh ../config.sh $(OBJDIR)gtkdoc/example.types
	../../src/scripts/doc/gtkdoc.sh -P "$(PREFIX)" -- "$(OBJDIR)gtkdoc/tmpl.stamp"

$(OBJDIR)gtkdoc/xml.stamp: ../../src/scripts/doc/gtkdoc.sh ../config.sh $(OBJDIR)gtkdoc/tmpl.stamp
	../../src/scripts/doc/gtkdoc.sh -P "$(PREFIX)" -- "$(OBJDIR)gtkdoc/xml.stamp"

clean:
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/" clean; \
		else $(MAKE) clean; fi) || exit; done
	../../src/scripts/doc/gtkdoc.sh -c -P "$(PREFIX)" -- "$(OBJDIR)gtkdoc/example.types"
	../../src/scripts/doc/gtkdoc.sh -c -P "$(PREFIX)" -- "$(OBJDIR)gtkdoc/html.stamp"
	../../src/scripts/doc/gtkdoc.sh -c -P "$(PREFIX)" -- "$(OBJDIR)gtkdoc/tmpl.stamp"
	../../src/scripts/doc/gtkdoc.sh -c -P "$(PREFIX)" -- "$(OBJDIR)gtkdoc/xml.stamp"

distclean:
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/" distclean; \
		else $(MAKE) distclean; fi) || exit; done
	../../src/scripts/doc/gtkdoc.sh -c -P "$(PREFIX)" -- "$(OBJDIR)gtkdoc/example.types"
	../../src/scripts/doc/gtkdoc.sh -c -P "$(PREFIX)" -- "$(OBJDIR)gtkdoc/html.stamp"
	../../src/scripts/doc/gtkdoc.sh -c -P "$(PREFIX)" -- "$(OBJDIR)gtkdoc/tmpl.stamp"
	../../src/scripts/doc/gtkdoc.sh -c -P "$(PREFIX)" -- "$(OBJDIR)gtkdoc/xml.stamp"
	$(RM) -- $(TARGETS)

install: all
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/" install; \
		else $(MAKE) install; fi) || exit; done

uninstall:
	@for i in $(SUBDIRS); do (cd "$$i" && \
		if [ -n "$(OBJDIR)" ]; then \
		$(MAKE) OBJDIR="$(OBJDIR)$$i/" uninstall; \
		else $(MAKE) uninstall; fi) || exit; done

.PHONY: all subdirs clean distclean install uninstall
