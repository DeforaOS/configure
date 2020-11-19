TARGETS	= $(OBJDIR)configure.1 $(OBJDIR)configure.html $(OBJDIR)project.conf.5 $(OBJDIR)project.conf.html
OBJDIR	=
PREFIX	= /usr/local
DESTDIR	=
RM	= rm -f
LN	= ln -f
MKDIR	= mkdir -m 0755 -p
INSTALL	= install


all: $(TARGETS)

$(OBJDIR)configure.1: ../src/scripts/doc/docbook.sh configure.xml
	../src/scripts/doc/docbook.sh -P "$(PREFIX)" -- "$(OBJDIR)configure.1"

$(OBJDIR)configure.html: ../src/scripts/doc/docbook.sh configure.css.xml configure.xml manual.css.xml
	../src/scripts/doc/docbook.sh -P "$(PREFIX)" -- "$(OBJDIR)configure.html"

$(OBJDIR)project.conf.5: ../src/scripts/doc/docbook.sh project.conf.xml
	../src/scripts/doc/docbook.sh -P "$(PREFIX)" -- "$(OBJDIR)project.conf.5"

$(OBJDIR)project.conf.html: ../src/scripts/doc/docbook.sh project.conf.css.xml project.conf.xml manual.css.xml
	../src/scripts/doc/docbook.sh -P "$(PREFIX)" -- "$(OBJDIR)project.conf.html"

clean:
	../src/scripts/doc/docbook.sh -c -P "$(PREFIX)" -- "$(OBJDIR)configure.1"
	../src/scripts/doc/docbook.sh -c -P "$(PREFIX)" -- "$(OBJDIR)configure.html"
	../src/scripts/doc/docbook.sh -c -P "$(PREFIX)" -- "$(OBJDIR)project.conf.5"
	../src/scripts/doc/docbook.sh -c -P "$(PREFIX)" -- "$(OBJDIR)project.conf.html"

distclean: clean
	$(RM) -- $(TARGETS)

install: all
	../src/scripts/doc/docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "$(OBJDIR)configure.1"
	../src/scripts/doc/docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "$(OBJDIR)configure.html"
	../src/scripts/doc/docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "$(OBJDIR)project.conf.5"
	../src/scripts/doc/docbook.sh -P "$(DESTDIR)$(PREFIX)" -i -- "$(OBJDIR)project.conf.html"

uninstall:
	../src/scripts/doc/docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "configure.1"
	../src/scripts/doc/docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "configure.html"
	../src/scripts/doc/docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "project.conf.5"
	../src/scripts/doc/docbook.sh -P "$(DESTDIR)$(PREFIX)" -u -- "project.conf.html"

.PHONY: all clean distclean install uninstall
