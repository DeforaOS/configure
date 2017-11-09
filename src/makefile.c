/* $Id$ */
/* Copyright (c) 2006-2017 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel configure */
/* All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */



#include <System.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>
#include "settings.h"
#include "configure.h"
#include "../config.h"

#ifndef PROGNAME
# define PROGNAME PACKAGE
#endif


/* private */
/* prototypes */
/* accessors */
static String const * _makefile_get_config(Configure * configure,
		String const * section, String const * variable);

static unsigned int _makefile_is_flag_set(Configure * configure,
		unsigned int flag);
static int _makefile_is_phony(Configure * configure, char const * target);

/* useful */
static int _makefile_expand(FILE * fp, char const * field);
static int _makefile_link(FILE * fp, int symlink, char const * link,
		char const * path);
static int _makefile_output_program(Configure * configure, FILE * fp,
		String const * name);
static int _makefile_output_variable(FILE * fp, String const * name,
		String const * value);
static int _makefile_mkdir(FILE * fp, char const * directory);
static int _makefile_print(FILE * fp, char const * format, ...);
static int _makefile_remove(FILE * fp, int recursive, ...);
static int _makefile_subdirs(FILE * fp, char const * target);
static int _makefile_target(FILE * fp, char const * target, ...);
#ifdef WITH_UNUSED
static int _makefile_targetv(FILE * fp, char const * target,
		char const ** depends);
#endif


/* functions */
/* makefile */
static int _makefile_write(Configure * configure, FILE * fp, configArray * ca,
	       	int from, int to);

int makefile(Configure * configure, String const * directory, configArray * ca,
	       	int from, int to)
		
{
	String * makefile;
	FILE * fp = NULL;
	int ret = 0;

	if((makefile = string_new_append(directory, "/", MAKEFILE, NULL))
			== NULL)
		return -1;
	if(!_makefile_is_flag_set(configure, PREFS_n)
			&& (fp = fopen(makefile, "w")) == NULL)
		ret = configure_error(makefile, 1);
	else
	{
		if(_makefile_is_flag_set(configure, PREFS_v))
			printf("%s%s/%s", "Creating ", directory,
					MAKEFILE "\n");
		ret |= _makefile_write(configure, fp, ca, from, to);
		if(fp != NULL)
			fclose(fp);
	}
	string_delete(makefile);
	return ret;
}

static int _write_variables(Configure * configure, FILE * fp);
static int _write_targets(Configure * configure, FILE * fp);
static int _write_objects(Configure * configure, FILE * fp);
static int _write_clean(Configure * configure, FILE * fp);
static int _write_distclean(Configure * configure, FILE * fp);
static int _write_dist(Configure * configure, FILE * fp, configArray * ca,
	       	int from, int to);
static int _write_distcheck(Configure * configure, FILE * fp);
static int _write_install(Configure * configure, FILE * fp);
static int _write_phony(Configure * configure, FILE * fp,
		char const ** targets);
static int _write_uninstall(Configure * configure, FILE * fp);
static int _makefile_write(Configure * configure, FILE * fp, configArray * ca,
	       	int from, int to)
{
	char const * depends[9] = { "all" };
	size_t i = 1;

	if(_write_variables(configure, fp) != 0
			|| _write_targets(configure, fp) != 0
			|| _write_objects(configure, fp) != 0
			|| _write_clean(configure, fp) != 0
			|| _write_distclean(configure, fp) != 0
			|| _write_dist(configure, fp, ca, from, to) != 0
			|| _write_distcheck(configure, fp) != 0
			|| _write_install(configure, fp) != 0
			|| _write_uninstall(configure, fp) != 0)
		return 1;
	if(_makefile_get_config(configure, NULL, "subdirs") != NULL)
		depends[i++] = "subdirs";
	depends[i++] = "clean";
	depends[i++] = "distclean";
	if(_makefile_get_config(configure, NULL, "package") != NULL
			&& _makefile_get_config(configure, NULL, "version") != NULL)
	{
		depends[i++] = "dist";
		depends[i++] = "distcheck";
	}
	depends[i++] = "install";
	depends[i++] = "uninstall";
	depends[i++] = NULL;
	return _write_phony(configure, fp, depends);
}

static int _variables_package(Configure * configure, FILE * fp,
		String const * directory);
static int _variables_print(Configure * configure, FILE * fp,
	       	char const * input, char const * output);
static int _variables_dist(Configure * configure, FILE * fp);
static int _variables_targets(Configure * configure, FILE * fp);
static int _variables_targets_library(Configure * configure, FILE * fp,
		char const * target);
static int _variables_executables(Configure * configure, FILE * fp);
static int _variables_includes(Configure * configure, FILE * fp);
static int _variables_subdirs(Configure * configure, FILE * fp);
static int _write_variables(Configure * configure, FILE * fp)
{
	int ret = 0;
	String const * directory;

	directory = _makefile_get_config(configure, NULL, "directory");
	ret |= _variables_package(configure, fp, directory);
	ret |= _variables_print(configure, fp, "subdirs", "SUBDIRS");
	ret |= _variables_dist(configure, fp);
	ret |= _variables_targets(configure, fp);
	ret |= _variables_executables(configure, fp);
	ret |= _variables_includes(configure, fp);
	ret |= _variables_subdirs(configure, fp);
	_makefile_print(fp, "%c", '\n');
	return ret;
}

static int _variables_package(Configure * configure, FILE * fp,
		String const * directory)
{
	String const * package;
	String const * version;
	String const * p;

	if((package = _makefile_get_config(configure, NULL, "package")) == NULL)
		return 0;
	if(_makefile_is_flag_set(configure, PREFS_v))
		printf("%s%s", "Package: ", package);
	if((version = _makefile_get_config(configure, NULL, "version")) == NULL)
	{
		if(_makefile_is_flag_set(configure, PREFS_v))
			fputc('\n', stdout);
		fprintf(stderr, "%s%s%s", PROGNAME ": ", directory,
				": \"package\" needs \"version\"\n");
		return 1;
	}
	if(_makefile_is_flag_set(configure, PREFS_v))
		printf(" %s\n", version);
	_makefile_output_variable(fp, "PACKAGE", package);
	_makefile_output_variable(fp, "VERSION", version);
	if((p = _makefile_get_config(configure, NULL, "config")) != NULL)
		return settings(configure, directory, package, version);
	return 0;
}

static int _variables_print(Configure * configure, FILE * fp,
		char const * input, char const * output)
{
	String const * p;
	String * prints;
	String * q;
	unsigned long i;
	char c;

	if((p = _makefile_get_config(configure, NULL, input)) == NULL)
		return 0;
	if((prints = string_new(p)) == NULL)
		return 1;
	q = prints;
	_makefile_print(fp, "%s%s", output, "\t=");
	for(i = 0;; i++)
	{
		if(prints[i] != ',' && prints[i] != '\0')
			continue;
		c = prints[i];
		prints[i] = '\0';
		if(strchr(prints, ' ') != NULL)
			_makefile_print(fp, " \"%s\"", prints);
		else
			_makefile_print(fp, " %s", prints);
		if(c == '\0')
			break;
		prints += i + 1;
		i = 0;
	}
	_makefile_print(fp, "%c", '\n');
	string_delete(q);
	return 0;
}

static int _variables_dist(Configure * configure, FILE * fp)
{
	ConfigurePrefs const * prefs;
	String const * p;
	String * dist;
	String * q;
	size_t i;
	char c;

	if((p = _makefile_get_config(configure, NULL, "dist")) == NULL)
		return 0;
	if((dist = string_new(p)) == NULL)
		return 1;
	prefs = configure_get_prefs(configure);
	q = dist;
	for(i = 0;; i++)
	{
		if(dist[i] != ',' && dist[i] != '\0')
			continue;
		c = dist[i];
		dist[i] = '\0';
		if(_makefile_get_config(configure, dist, "install") != NULL)
		{
			/* FIXME may still need to be output */
			if(_makefile_get_config(configure, NULL, "targets")
					== NULL)
			{
				_makefile_output_variable(fp, "OBJDIR", "");
				_makefile_output_variable(fp, "PREFIX",
						prefs->prefix);
				_makefile_output_variable(fp, "DESTDIR",
						prefs->destdir);
			}
			_makefile_output_program(configure, fp, "mkdir");
			_makefile_output_program(configure, fp, "install");
			_makefile_output_program(configure, fp, "rm");
			break;
		}
		if(c == '\0')
			break;
		dist += i + 1;
		i = 0;
	}
	string_delete(q);
	return 0;
}

static int _variables_targets(Configure * configure, FILE * fp)
{
	int ret = 0;
	String const * p;
	String * prints;
	String * q;
	size_t i;
	char c;
	String const * type;
	int phony;

	if((p = _makefile_get_config(configure, NULL, "targets")) == NULL)
		return 0;
	if((prints = string_new(p)) == NULL)
		return 1;
	q = prints;
	_makefile_print(fp, "%s", "TARGETS\t=");
	for(i = 0;; i++)
	{
		if(prints[i] != ',' && prints[i] != '\0')
			continue;
		c = prints[i];
		prints[i] = '\0';
		if((type = _makefile_get_config(configure, prints, "type"))
			       	== NULL)
			_makefile_print(fp, " %s", prints);
		else
			switch(enum_string(TT_LAST, sTargetType, type))
			{
				case TT_BINARY:
					_makefile_print(fp,
							" $(OBJDIR)%s$(EXEEXT)",
							prints);
					break;
				case TT_LIBRARY:
					ret |= _variables_targets_library(
							configure, fp, prints);
					break;
				case TT_LIBTOOL:
					_makefile_print(fp, " $(OBJDIR)%s%s",
							prints, ".la");
					break;
				case TT_OBJECT:
				case TT_UNKNOWN:
					_makefile_print(fp, " $(OBJDIR)%s",
							prints);
					break;
				case TT_SCRIPT:
					phony = _makefile_is_phony(configure,
							prints);
					_makefile_print(fp, " %s%s", phony
							? "" : "$(OBJDIR)",
							prints);
					break;
				case TT_PLUGIN:
					_makefile_print(fp,
							" $(OBJDIR)%s$(SOEXT)",
							prints);
					break;
			}
		if(c == '\0')
			break;
		prints += i + 1;
		i = 0;
	}
	_makefile_print(fp, "%c", '\n');
	string_delete(q);
	return ret;
}

static int _variables_targets_library(Configure * configure, FILE * fp,
		char const * target)
{
	String * soname;
	String const * p;

	if((p = _makefile_get_config(configure, target, "soname")) != NULL)
		soname = string_new(p);
	else if(configure_get_os(configure) == HO_MACOSX)
		/* versioning is different on MacOS X */
		soname = string_new_append(target, ".0.0$(SOEXT)", NULL);
	else if(configure_get_os(configure) == HO_WIN32)
		/* and on Windows */
		soname = string_new_append(target, "$(SOEXT)", NULL);
	else
		soname = string_new_append(target, "$(SOEXT)", ".0", NULL);
	if(soname == NULL)
		return 1;
	if(configure_can_library_static(configure))
		/* generate a static library */
		_makefile_print(fp, "%s%s%s", " $(OBJDIR)", target, ".a");
	if(configure_get_os(configure) == HO_MACOSX)
		_makefile_print(fp, "%s%s%s%s%s%s%s", " $(OBJDIR)", soname,
				" $(OBJDIR)", target, ".0$(SOEXT) $(OBJDIR)",
				target, "$(SOEXT)");
	else if(configure_get_os(configure) == HO_WIN32)
		_makefile_print(fp, "%s%s", " $(OBJDIR)", soname);
	else
		_makefile_print(fp, "%s%s%s%s%s%s%s", " $(OBJDIR)", soname,
				".0 $(OBJDIR)", soname, " $(OBJDIR)", target,
				"$(SOEXT)");
	string_delete(soname);
	return 0;
}

static void _executables_variables(Configure * configure, FILE * fp,
	       	String const * target, char * done);
static int _variables_executables(Configure * configure, FILE * fp)
{
	ConfigurePrefs const * prefs;
	char done[TT_LAST]; /* FIXME even better if'd be variable by variable */
	String const * targets;
	String const * includes;
	String const * package;
	String * p;
	String * q;
	size_t i;
	char c;

	prefs = configure_get_prefs(configure);
	memset(&done, 0, sizeof(done));
	targets = _makefile_get_config(configure, NULL, "targets");
	includes = _makefile_get_config(configure, NULL, "includes");
	package = _makefile_get_config(configure, NULL, "package");
	if(targets != NULL)
	{
		if((p = string_new(targets)) == NULL)
			return 1;
		q = p;
		for(i = 0;; i++)
		{
			if(p[i] != ',' && p[i] != '\0')
				continue;
			c = p[i];
			p[i] = '\0';
			_executables_variables(configure, fp, p, done);
			if(c == '\0')
				break;
			p += i + 1;
			i = 0;
		}
		string_delete(q);
	}
	else if(includes != NULL)
	{
		_makefile_output_variable(fp, "OBJDIR", "");
		_makefile_output_variable(fp, "PREFIX", prefs->prefix);
		_makefile_output_variable(fp, "DESTDIR", prefs->destdir);
	}
	if(targets != NULL || includes != NULL || package != NULL)
	{
		_makefile_output_program(configure, fp, "rm");
		_makefile_output_program(configure, fp, "ln");
	}
	if(package != NULL)
	{
		_makefile_output_program(configure, fp, "tar");
		_makefile_output_program(configure, fp, "mkdir");
	}
	if(targets != NULL || includes != NULL)
	{
		if(package == NULL)
			_makefile_output_program(configure, fp, "mkdir");
		_makefile_output_program(configure, fp, "install");
	}
	return 0;
}

static void _variables_binary(Configure * configure, FILE * fp, char * done);
static void _variables_library(Configure * configure, FILE * fp, char * done);
static void _variables_library_static(Configure * configure, FILE * fp);
static void _variables_libtool(Configure * configure, FILE * fp, char * done);
static void _variables_script(Configure * configure, FILE * fp, char * done);
static void _executables_variables(Configure * configure, FILE * fp,
	       	String const * target, char * done)
{
	String const * type;
	TargetType tt;

	if((type = _makefile_get_config(configure, target, "type")) == NULL)
		return;
	if(done[(tt = enum_string(TT_LAST, sTargetType, type))])
		return;
	switch(tt)
	{
		case TT_BINARY:
			_variables_binary(configure, fp, done);
			done[TT_OBJECT] = 1;
			break;
		case TT_OBJECT:
			_variables_binary(configure, fp, done);
			done[TT_BINARY] = 1;
			break;
		case TT_LIBRARY:
			_variables_library(configure, fp, done);
			done[TT_PLUGIN] = 1;
			break;
		case TT_PLUGIN:
			_variables_library(configure, fp, done);
			done[TT_LIBRARY] = 1;
			break;
		case TT_LIBTOOL:
			_variables_libtool(configure, fp, done);
			break;
		case TT_SCRIPT:
			_variables_script(configure, fp, done);
			break;
		case TT_UNKNOWN:
			break;
	}
	done[tt] = 1;
	return;
}

static void _targets_asflags(Configure * configure, FILE * fp);
static void _targets_cflags(Configure * configure, FILE * fp);
static void _targets_cxxflags(Configure * configure, FILE * fp);
static void _targets_exeext(Configure * configure, FILE * fp);
static void _targets_ldflags(Configure * configure, FILE * fp);
static void _targets_vflags(Configure * configure, FILE * fp);
static void _binary_ldflags(Configure * configure, FILE * fp,
		String const * ldflags);
static void _variables_binary(Configure * configure, FILE * fp, char * done)
{
	ConfigurePrefs const * prefs;
	String * p;

	prefs = configure_get_prefs(configure);
	if(!done[TT_LIBRARY] && !done[TT_SCRIPT])
	{
		_makefile_output_variable(fp, "OBJDIR", "");
		_makefile_output_variable(fp, "PREFIX", prefs->prefix);
		_makefile_output_variable(fp, "DESTDIR", prefs->destdir);
	}
	/* BINDIR */
	if(prefs->bindir[0] == '/')
		_makefile_output_variable(fp, "BINDIR",
				prefs->bindir);
	else if((p = string_new_append("$(PREFIX)/", prefs->bindir,
					NULL)) != NULL)
	{
		_makefile_output_variable(fp, "BINDIR", p);
		string_delete(p);
	}
	/* SBINDIR */
	if(prefs->sbindir[0] == '/')
		_makefile_output_variable(fp, "SBINDIR",
				prefs->sbindir);
	else if((p = string_new_append("$(PREFIX)/", prefs->sbindir,
					NULL)) != NULL)
	{
		_makefile_output_variable(fp, "SBINDIR", p);
		string_delete(p);
	}
	if(!done[TT_LIBRARY])
	{
		_targets_asflags(configure, fp);
		_targets_cflags(configure, fp);
		_targets_cxxflags(configure, fp);
		_targets_ldflags(configure, fp);
		_targets_vflags(configure, fp);
		_targets_exeext(configure, fp);
	}
}

static void _targets_asflags(Configure * configure, FILE * fp)
{
	String const * as;
	String const * asf;
	String const * asff;

	as = _makefile_get_config(configure, NULL, "as");
	asff = _makefile_get_config(configure, NULL, "asflags_force");
	asf = _makefile_get_config(configure, NULL, "asflags");
	if(as != NULL || asff != NULL || asf != NULL)
	{
		_makefile_output_variable(fp, "AS", (as != NULL) ? as
				: configure_get_program(configure, "as"));
		_makefile_output_variable(fp, "ASFLAGSF", asff);
		_makefile_output_variable(fp, "ASFLAGS", asf);
	}
}

static void _targets_cflags(Configure * configure, FILE * fp)
{
	String const * cc;
	String const * cff;
	String const * cf;
	String const * cppf;
	String const * cpp;
	String * p;

	cppf = _makefile_get_config(configure, NULL, "cppflags_force");
	cpp = _makefile_get_config(configure, NULL, "cppflags");
	cff = _makefile_get_config(configure, NULL, "cflags_force");
	cf = _makefile_get_config(configure, NULL, "cflags");
	cc = _makefile_get_config(configure, NULL, "cc");
	if(cppf == NULL && cpp == NULL && cff == NULL && cf == NULL
			&& cc == NULL)
		return;
	if(cc == NULL)
		_makefile_output_program(configure, fp, "cc");
	else
		_makefile_output_variable(fp, "CC", cc);
	_makefile_output_variable(fp, "CPPFLAGSF", cppf);
	_makefile_output_variable(fp, "CPPFLAGS", cpp);
	p = NULL;
	if(configure_get_os(configure) == HO_GNU_LINUX && cff != NULL
			&& string_find(cff, "-ansi"))
		p = string_new_append(cff, " -D _GNU_SOURCE");
	_makefile_output_variable(fp, "CFLAGSF", (p != NULL) ? p : cff);
	string_delete(p);
	p = NULL;
	if(configure_get_os(configure) == HO_GNU_LINUX && cf != NULL
			&& string_find(cf, "-ansi"))
		p = string_new_append(cf, " -D _GNU_SOURCE");
	_makefile_output_variable(fp, "CFLAGS", (p != NULL) ? p : cf);
	string_delete(p);
}

static void _targets_cxxflags(Configure * configure, FILE * fp)
{
	String const * cxx;
	String const * cxxff;
	String const * cxxf;

	cxx = _makefile_get_config(configure, NULL, "cxx");
	cxxff = _makefile_get_config(configure, NULL, "cxxflags_force");
	cxxf = _makefile_get_config(configure, NULL, "cxxflags");
	if(cxx != NULL || cxxff != NULL || cxxf != NULL)
	{
		if(cxx == NULL)
			cxx = configure_get_program(configure, "cxx");
		_makefile_output_variable(fp, "CXX", cxx);
	}
	if(cxxff != NULL)
	{
		_makefile_print(fp, "%s%s", "CXXFLAGSF= ", cxxff);
		if(configure_get_os(configure) == HO_GNU_LINUX
				&& string_find(cxxff, "-ansi"))
			_makefile_print(fp, "%s", " -D _GNU_SOURCE");
		_makefile_print(fp, "%c", '\n');
	}
	if(cxxf != NULL)
	{
		_makefile_print(fp, "%s%s", "CXXFLAGS= ", cxxf);
		if(configure_get_os(configure) == HO_GNU_LINUX
				&& string_find(cxxf, "-ansi"))
			_makefile_print(fp, "%s", " -D _GNU_SOURCE");
		_makefile_print(fp, "%c", '\n');
	}
}

static void _targets_exeext(Configure * configure, FILE * fp)
{
	_makefile_output_variable(fp, "EXEEXT",
			configure_get_exeext(configure));
}

static void _targets_ldflags(Configure * configure, FILE * fp)
{
	String const * p;

	if((p = _makefile_get_config(configure, NULL, "ldflags_force")) != NULL)
	{
		_makefile_print(fp, "%s", "LDFLAGSF=");
		_binary_ldflags(configure, fp, p);
		_makefile_print(fp, "%c", '\n');
	}
	if((p = _makefile_get_config(configure, NULL, "ldflags")) != NULL)
	{
		_makefile_print(fp, "%s", "LDFLAGS\t=");
		_binary_ldflags(configure, fp, p);
		_makefile_print(fp, "%c", '\n');
	}
}

static void _targets_vflags(Configure * configure, FILE * fp)
{
	String const * p;

	if((p = _makefile_get_config(configure, NULL, "verilog")) != NULL)
		_makefile_output_variable(fp, "VERILOG", p);
	if((p = _makefile_get_config(configure, NULL, "vflags_force")) != NULL)
	{
		_makefile_print(fp, "%s", "VFLAGSF=");
		_makefile_print(fp, "%c", '\n');
	}
	if((p = _makefile_get_config(configure, NULL, "vflags")) != NULL)
	{
		_makefile_print(fp, "%s", "VFLAGS\t=");
		_makefile_print(fp, "%c", '\n');
	}
}

static void _binary_ldflags(Configure * configure, FILE * fp,
		String const * ldflags)
{
	char const * libs_bsd[] = { "dl", "resolv", "ossaudio", "socket",
		"ws2_32", NULL };
	char const * libs_deforaos[] = { "ossaudio", "resolv", "ssl", "ws2_32",
		NULL };
	char const * libs_gnu[] = { "intl", "ossaudio", "resolv", "socket",
		"ws2_32", NULL };
	char const * libs_macosx[] = { "crypt", "ossaudio", "socket", "ws2_32",
		NULL };
	char const * libs_netbsd[] = { "dl", "resolv", "socket", "ws2_32",
		NULL };
	char const * libs_sunos[] = { "dl", "ossaudio", "ws2_32", NULL };
	char const * libs_win32[] = { "dl", "ossaudio", NULL };
	char buf[16];
	char const ** libs;
	String * p;
	String * q;
	size_t i;

	if((p = string_new(ldflags)) == NULL) /* XXX report error? */
	{
		_makefile_print(fp, " %s%s", ldflags, "\n");
		return;
	}
	switch(configure_get_os(configure))
	{
		case HO_DEFORAOS:
			libs = libs_deforaos;
			break;
		case HO_FREEBSD:
		case HO_OPENBSD:
			libs = libs_bsd;
			break;
		case HO_GNU_LINUX:
			libs = libs_gnu;
			break;
		case HO_MACOSX:
			libs = libs_macosx;
			break;
		case HO_NETBSD:
			libs = libs_netbsd;
			break;
		case HO_SUNOS:
			libs = libs_sunos;
			break;
		case HO_WIN32:
			libs = libs_win32;
			break;
		default:
			libs = libs_gnu;
			break;
	}
	for(i = 0; libs[i] != NULL; i++)
	{
		snprintf(buf, sizeof(buf), "-l %s", libs[i]);
		if((q = string_find(p, buf)) == NULL)
		{
			snprintf(buf, sizeof(buf), "-l%s", libs[i]);
			q = string_find(p, buf);
		}
		if(q == NULL)
			continue;
		memmove(q, q + strlen(buf), strlen(q) - strlen(buf) + 1);
	}
	_makefile_print(fp, " %s", p);
	string_delete(p);
}

static void _variables_library(Configure * configure, FILE * fp, char * done)
{
	ConfigurePrefs const * prefs;
	String const * libdir;
	String const * p;

	prefs = configure_get_prefs(configure);
	if(!done[TT_LIBRARY] && !done[TT_SCRIPT])
	{
		_makefile_output_variable(fp, "OBJDIR", "");
		_makefile_output_variable(fp, "PREFIX", prefs->prefix);
		_makefile_output_variable(fp, "DESTDIR", prefs->destdir);
	}
	if((libdir = _makefile_get_config(configure, NULL, "libdir")) == NULL)
		libdir = prefs->libdir;
	if(libdir[0] == '/')
		_makefile_output_variable(fp, "LIBDIR", libdir);
	else
		_makefile_print(fp, "%s%s\n", "LIBDIR\t= $(PREFIX)/", libdir);
	if(!done[TT_BINARY])
	{
		_targets_asflags(configure, fp);
		_targets_cflags(configure, fp);
		_targets_cxxflags(configure, fp);
		_targets_ldflags(configure, fp);
		_targets_vflags(configure, fp);
		_targets_exeext(configure, fp);
	}
	if(configure_can_library_static(configure))
		_variables_library_static(configure, fp);
	if((p = _makefile_get_config(configure, NULL, "ld")) == NULL)
		_makefile_output_program(configure, fp, "ccshared");
	else
		_makefile_output_variable(fp, "CCSHARED", p);
	_makefile_output_variable(fp, "SOEXT", configure_get_soext(configure));
}

static void _variables_library_static(Configure * configure, FILE * fp)
{
	String const * p;

	if((p = _makefile_get_config(configure, NULL, "ar")) == NULL)
		_makefile_output_program(configure, fp, "ar");
	else
		_makefile_output_variable(fp, "AR", p);
	_makefile_output_variable(fp, "ARFLAGS", "-rc");
	if((p = _makefile_get_config(configure, NULL, "ranlib")) == NULL)
		_makefile_output_program(configure, fp, "ranlib");
	else
		_makefile_output_variable(fp, "RANLIB", p);
}

static void _variables_libtool(Configure * configure, FILE * fp, char * done)
{
	String const * p;

	_variables_library(configure, fp, done);
	if(!done[TT_LIBTOOL])
	{
		if((p = _makefile_get_config(configure, NULL, "libtool"))
				== NULL)
			_makefile_output_program(configure, fp, "libtool");
		else
			_makefile_output_variable(fp, "LIBTOOL", p);
	}
}

static void _variables_script(Configure * configure, FILE * fp, char * done)
{
	ConfigurePrefs const * prefs;

	if(!done[TT_BINARY] && !done[TT_LIBRARY] && !done[TT_SCRIPT])
	{
		prefs = configure_get_prefs(configure);
		_makefile_output_variable(fp, "OBJDIR", "");
		_makefile_output_variable(fp, "PREFIX", prefs->prefix);
		_makefile_output_variable(fp, "DESTDIR", prefs->destdir);
	}
}

static int _variables_includes(Configure * configure, FILE * fp)
{
	ConfigurePrefs const * prefs;
	String const * includes;

	if((includes = _makefile_get_config(configure, NULL, "includes"))
			== NULL)
		return 0;
	if(fp == NULL)
		return 0;
	prefs = configure_get_prefs(configure);
	if(prefs->includedir[0] == '/')
		_makefile_output_variable(fp, "INCLUDEDIR", prefs->includedir);
	else
		_makefile_print(fp, "%s%s\n", "INCLUDEDIR= $(PREFIX)/",
				prefs->includedir);
	return 0;
}

static int _variables_subdirs(Configure * configure, FILE * fp)
{
	String const * sections[] = { "dist" };
	size_t i;
	String * p;
	String const * q;

	if(_makefile_get_config(configure, NULL, "subdirs") == NULL
			|| _makefile_get_config(configure, NULL, "package") != NULL
			|| _makefile_get_config(configure, NULL, "targets") != NULL
			|| _makefile_get_config(configure, NULL, "includes") != NULL)
		return 0;
	for(i = 0; i < sizeof(sections) / sizeof(*sections); i++)
	{
		if((q = _makefile_get_config(configure, NULL, sections[i]))
				== NULL)
			continue;
		if((p = strdup(q)) == NULL)
			return -1;
		for(q = strtok(p, ","); q != NULL; q = strtok(NULL, ","))
			if(_makefile_get_config(configure, q, "install")
					!= NULL)
				break;
		free(p);
		if(q != NULL)
			return 0;
	}
	return _makefile_output_program(configure, fp, "mkdir");
}

static int _targets_all(Configure * configure, FILE * fp);
static int _targets_subdirs(Configure * configure, FILE * fp);
static int _targets_target(Configure * configure, FILE * fp,
		String const * target);
static int _write_targets(Configure * configure, FILE * fp)
{
	int ret = 0;
	String const * p;
	String * targets;
	String * q;
	size_t i;
	char c;

	if(_targets_all(configure, fp) != 0
			|| _targets_subdirs(configure, fp) != 0)
		return 1;
	if((p = _makefile_get_config(configure, NULL, "targets")) == NULL)
		return 0;
	if((targets = string_new(p)) == NULL)
		return 1;
	q = targets;
	for(i = 0;; i++)
	{
		if(targets[i] != ',' && targets[i] != '\0')
			continue;
		c = targets[i];
		targets[i] = '\0';
		ret |= _targets_target(configure, fp, targets);
		if(c == '\0')
			break;
		targets += i + 1;
		i = 0;
	}
	string_delete(q);
	return ret;
}

static int _targets_all(Configure * configure, FILE * fp)
{
	char const * depends[] = { NULL, NULL };
	size_t i = 0;

	if(_makefile_get_config(configure, NULL, "subdirs") != NULL)
		depends[i++] = "subdirs";
	if(_makefile_get_config(configure, NULL, "targets") != NULL)
		depends[i++] = "$(TARGETS)";
	_makefile_target(fp, "all", depends[0], depends[1], NULL);
	return 0;
}

static int _targets_subdirs(Configure * configure, FILE * fp)
{
	String const * subdirs;

	if((subdirs = _makefile_get_config(configure, NULL, "subdirs")) != NULL)
	{
		_makefile_target(fp, "subdirs", NULL);
		_makefile_subdirs(fp, NULL);
	}
	return 0;
}

static int _target_objs(Configure * configure, FILE * fp,
		String const * target);
static int _target_binary(Configure * configure, FILE * fp,
		String const * target);
static int _target_library(Configure * configure, FILE * fp,
		String const * target);
static int _target_library_static(Configure * configure, FILE * fp,
		String const * target);
static int _target_libtool(Configure * configure, FILE * fp,
		String const * target);
static int _target_object(Configure * configure, FILE * fp,
		String const * target);
static int _target_plugin(Configure * configure, FILE * fp,
		String const * target);
static int _target_script(Configure * configure, FILE * fp,
		String const * target);
static int _targets_target(Configure * configure, FILE * fp,
		String const * target)
{
	String const * type;
	TargetType tt;

	if((type = _makefile_get_config(configure, target, "type")) == NULL)
	{
		fprintf(stderr, "%s%s%s", PROGNAME ": ", target,
				": no type defined for target\n");
		return 1;
	}
	tt = enum_string(TT_LAST, sTargetType, type);
	switch(tt)
	{
		case TT_BINARY:
			return _target_binary(configure, fp, target);
		case TT_LIBRARY:
			return _target_library(configure, fp, target);
		case TT_LIBTOOL:
			return _target_libtool(configure, fp, target);
		case TT_OBJECT:
			return _target_object(configure, fp, target);
		case TT_PLUGIN:
			return _target_plugin(configure, fp, target);
		case TT_SCRIPT:
			return _target_script(configure, fp, target);
		case TT_UNKNOWN:
			fprintf(stderr, "%s%s%s", PROGNAME ": ", target,
					": unknown type for target\n");
			return 1;
	}
	return 0;
}

static int _objs_source(FILE * fp, String * source, TargetType tt);
static int _target_objs(Configure * configure, FILE * fp,
		String const * target)
{
	int ret = 0;
	String const * p;
	TargetType tt = TT_UNKNOWN;
	String * sources;
	String * q;
	size_t i;
	char c;

	if((p = _makefile_get_config(configure, target, "type")) != NULL)
		tt = enum_string(TT_LAST, sTargetType, p);
	if((p = _makefile_get_config(configure, target, "sources")) == NULL)
		/* a binary target may have no sources */
		return 0;
	if((sources = string_new(p)) == NULL)
		return 1;
	q = sources;
	_makefile_print(fp, "%s%s%s", "\n", target, "_OBJS =");
	for(i = 0; ret == 0; i++)
	{
		if(sources[i] != ',' && sources[i] != '\0')
			continue;
		c = sources[i];
		sources[i] = '\0';
		ret = _objs_source(fp, sources, tt);
		if(c == '\0')
			break;
		sources += i + 1;
		i = 0;
	}
	_makefile_print(fp, "%c", '\n');
	string_delete(q);
	return ret;
}

static int _objs_source(FILE * fp, String * source, TargetType tt)
{
	int ret = 0;
	String const * extension;
	size_t len;

	if((extension = _source_extension(source)) == NULL)
	{
		fprintf(stderr, "%s%s%s", PROGNAME ": ", source,
				": no extension for source\n");
		return 1;
	}
	len = string_length(source) - string_length(extension) - 1;
	source[len] = '\0';
	switch(_source_type(extension))
	{
		case OT_ASM_SOURCE:
		case OT_C_SOURCE:
		case OT_CXX_SOURCE:
		case OT_OBJC_SOURCE:
		case OT_OBJCXX_SOURCE:
			_makefile_print(fp, "%s%s%s", " $(OBJDIR)", source,
					(tt == TT_LIBTOOL) ? ".lo" : ".o");
			break;
		case OT_VERILOG_SOURCE:
			_makefile_print(fp, "%s%s%s", " $(OBJDIR)", source,
					".o");
			break;
		case OT_UNKNOWN:
			ret = 1;
			fprintf(stderr, "%s%s%s", PROGNAME ": ", source,
					": unknown extension for source\n");
			break;
	}
	source[len] = '.';
	return ret;
}

static int _target_flags(Configure * configure, FILE * fp,
		String const * target);
static int _target_binary(Configure * configure, FILE * fp,
		String const * target)
{
	String const * p;

	if(_target_objs(configure, fp, target) != 0)
		return 1;
	if(_target_flags(configure, fp, target) != 0)
		return 1;
	_makefile_print(fp, "%c", '\n');
	/* output the binary target */
	_makefile_print(fp, "%s%s%s%s%s%s", "$(OBJDIR)", target, "$(EXEEXT)",
			": $(", target, "_OBJS)");
	if((p = _makefile_get_config(configure, target, "depends")) != NULL
			&& _makefile_expand(fp, p) != 0)
		return error_print(PROGNAME);
	_makefile_print(fp, "%c", '\n');
	/* build the binary */
	_makefile_print(fp, "%s%s%s%s%s%s%s", "\t$(CC) -o $(OBJDIR)", target,
			"$(EXEEXT) $(", target, "_OBJS) $(", target,
			"_LDFLAGS)\n");
	return 0;
}

static void _flags_asm(Configure * configure, FILE * fp, String const * target);
static void _flags_c(Configure * configure, FILE * fp, String const * target);
static void _flags_cxx(Configure * configure, FILE * fp, String const * target);
static void _flags_verilog(Configure * configure, FILE * fp,
		String const * target);
static int _target_flags(Configure * configure, FILE * fp,
		String const * target)
{
	char done[OT_COUNT];
	String const * p;
	String * sources;
	String * q;
	String const * extension;
	ObjectType type;
	char c;
	size_t i;

	memset(&done, 0, sizeof(done));
	if((p = _makefile_get_config(configure, target, "sources")) == NULL
			|| string_length(p) == 0)
	{
		if((p = _makefile_get_config(configure, target, "type")) != NULL
				&& string_compare(p, "binary") == 0)
			_flags_c(configure, fp, target);
		return 0;
	}
	if((sources = string_new(p)) == NULL)
		return 1;
	q = sources;
	for(i = 0;; i++)
	{
		if(sources[i] != ',' && sources[i] != '\0')
			continue;
		c = sources[i];
		sources[i] = '\0';
		extension = _source_extension(sources);
		if(extension == NULL)
		{
			sources[i] = c;
			continue;
		}
		type = _source_type(extension);
		if(!done[type])
			switch(type)
			{
				case OT_ASM_SOURCE:
					_flags_asm(configure, fp, target);
					break;
				case OT_OBJC_SOURCE:
					done[OT_C_SOURCE] = 1;
					/* fallback */
				case OT_C_SOURCE:
					done[OT_OBJC_SOURCE] = 1;
					_flags_c(configure, fp, target);
					break;
				case OT_OBJCXX_SOURCE:
					done[OT_CXX_SOURCE] = 1;
					/* fallback */
				case OT_CXX_SOURCE:
					done[OT_OBJCXX_SOURCE] = 1;
					_flags_cxx(configure, fp, target);
					break;
				case OT_VERILOG_SOURCE:
					done[OT_VERILOG_SOURCE] = 1;
					_flags_verilog(configure, fp, target);
					break;
				case OT_UNKNOWN:
					break;
			}
		done[type] = 1;
		if(c == '\0')
			break;
		sources += i + 1;
		i = 0;
	}
	string_delete(q);
	return 0;
}

static void _flags_asm(Configure * configure, FILE * fp, String const * target)
{
	String const * p;

	_makefile_print(fp, "%s%s", target, "_ASFLAGS = $(CPPFLAGSF)"
			" $(CPPFLAGS) $(ASFLAGSF) $(ASFLAGS)");
	if((p = _makefile_get_config(configure, target, "asflags")) != NULL)
		_makefile_print(fp, " %s", p);
	_makefile_print(fp, "%c", '\n');
}

static void _flags_c(Configure * configure, FILE * fp, String const * target)
{
	String const * p;

	_makefile_print(fp, "%s%s", target, "_CFLAGS = $(CPPFLAGSF)"
			" $(CPPFLAGS)");
	if((p = _makefile_get_config(configure, target, "cppflags")) != NULL)
		_makefile_print(fp, " %s", p);
	_makefile_print(fp, "%s", " $(CFLAGSF) $(CFLAGS)");
	if((p = _makefile_get_config(configure, target, "cflags")) != NULL)
	{
		_makefile_print(fp, " %s", p);
		if(configure_get_os(configure) == HO_GNU_LINUX
				&& string_find(p, "-ansi"))
			_makefile_print(fp, "%s", " -D _GNU_SOURCE");
	}
	_makefile_print(fp, "\n%s%s", target,
			"_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)");
	if((p = _makefile_get_config(configure, target, "ldflags")) != NULL)
		_binary_ldflags(configure, fp, p);
	_makefile_print(fp, "%c", '\n');
}

static void _flags_cxx(Configure * configure, FILE * fp, String const * target)
{
	String const * p;

	_makefile_print(fp, "%s%s", target, "_CXXFLAGS = $(CPPFLAGSF)"
			" $(CPPFLAGS) $(CXXFLAGSF) $(CXXFLAGS)");
	if((p = _makefile_get_config(configure, target, "cxxflags")) != NULL)
		_makefile_print(fp, " %s", p);
	_makefile_print(fp, "\n%s%s", target,
			"_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)");
	if((p = _makefile_get_config(configure, target, "ldflags")) != NULL)
		_binary_ldflags(configure, fp, p);
	_makefile_print(fp, "%c", '\n');
}

static void _flags_verilog(Configure * configure, FILE * fp,
		String const * target)
{
	String const * p;

	_makefile_print(fp, "%s%s", target, "_VFLAGS = $(VFLAGSF) $(VFLAGS)");
	if((p = _makefile_get_config(configure, target, "vflags")) != NULL)
		_makefile_print(fp, " %s", p);
	_makefile_print(fp, "%c", '\n');
}

static int _target_library(Configure * configure, FILE * fp,
		String const * target)
{
	String const * p;
	String * q;
	String * soname;

	if(_target_objs(configure, fp, target) != 0)
		return 1;
	if(_target_flags(configure, fp, target) != 0)
		return 1;
	if(configure_can_library_static(configure)
			/* generate a static library */
			&& _target_library_static(configure, fp, target) != 0)
		return 1;
	if((p = _makefile_get_config(configure, target, "soname")) != NULL)
		soname = string_new(p);
	else if(configure_get_os(configure) == HO_MACOSX)
		/* versioning is different on MacOS X */
		soname = string_new_append(target, ".0.0$(SOEXT)", NULL);
	else if(configure_get_os(configure) == HO_WIN32)
		/* and on Windows */
		soname = string_new_append(target, "$(SOEXT)", NULL);
	else
		soname = string_new_append(target, "$(SOEXT)", ".0", NULL);
	if(soname == NULL)
		return 1;
	if(configure_get_os(configure) == HO_MACOSX)
		_makefile_print(fp, "%s%s%s%s%s", "\n$(OBJDIR)", soname, ": $(",
				target, "_OBJS)");
	else if(configure_get_os(configure) == HO_WIN32)
		_makefile_print(fp, "%s%s%s%s%s", "\n$(OBJDIR)", soname, ": $(",
				target, "_OBJS)");
	else
		_makefile_print(fp, "%s%s%s%s%s", "\n$(OBJDIR)", soname,
				".0: $(", target, "_OBJS)");
	if((p = _makefile_get_config(configure, target, "depends")) != NULL
			&& _makefile_expand(fp, p) != 0)
		return error_print(PROGNAME);
	_makefile_print(fp, "%c", '\n');
	/* build the shared library */
	_makefile_print(fp, "%s%s%s", "\t$(CCSHARED) -o $(OBJDIR)", soname,
			(configure_get_os(configure) != HO_MACOSX
			 && configure_get_os(configure) != HO_WIN32)
			? ".0" : "");
	if((p = _makefile_get_config(configure, target, "install")) != NULL)
	{
		/* soname is not available on MacOS X */
		if(configure_get_os(configure) == HO_MACOSX)
			_makefile_print(fp, "%s%s%s%s%s", " -install_name ",
					p, "/", target, ".0$(SOEXT)");
		else if(configure_get_os(configure) != HO_WIN32)
			_makefile_print(fp, "%s%s", " -Wl,-soname,", soname);
	}
	_makefile_print(fp, "%s%s%s%s%s", " $(", target, "_OBJS) $(", target,
			"_LDFLAGS)");
	if((q = string_new_append(target, "$(SOEXT)", NULL)) == NULL)
	{
		string_delete(soname);
		return 1;
	}
	if((p = _makefile_get_config(configure, q, "ldflags")) != NULL)
		_binary_ldflags(configure, fp, p);
	string_delete(q);
	_makefile_print(fp, "%c", '\n');
	if(configure_get_os(configure) == HO_MACOSX)
	{
		_makefile_print(fp, "%s%s%s%s%s", "\n$(OBJDIR)", target,
				".0$(SOEXT): $(OBJDIR)", soname, "\n");
		_makefile_print(fp, "%s%s%s%s%s%s", "\t$(LN) -s -- ", soname,
				" $(OBJDIR)", target, ".0$(SOEXT)", "\n");
		_makefile_print(fp, "%s%s%s%s%s", "\n$(OBJDIR)", target,
				"$(SOEXT): $(OBJDIR)", soname, "\n");
		_makefile_print(fp, "%s%s%s%s%s", "\t$(LN) -s -- ", soname,
				" $(OBJDIR)", target, "$(SOEXT)\n");
	}
	else if(configure_get_os(configure) != HO_WIN32)
	{
		_makefile_print(fp, "%s%s%s%s%s", "\n$(OBJDIR)", soname,
				": $(OBJDIR)", soname, ".0\n");
		_makefile_print(fp, "%s%s%s%s%s", "\t$(LN) -s -- ", soname,
				".0 $(OBJDIR)", soname, "\n");
		_makefile_print(fp, "%s%s%s%s%s", "\n$(OBJDIR)", target,
				"$(SOEXT): $(OBJDIR)", soname, ".0\n");
		_makefile_print(fp, "%s%s%s%s%s", "\t$(LN) -s -- ", soname,
				".0 $(OBJDIR)", target, "$(SOEXT)\n");
	}
	string_delete(soname);
	return 0;
}

static int _target_library_static(Configure * configure, FILE * fp,
		String const * target)
{
	String const * p;
	String * q;
	size_t len;

	_makefile_print(fp, "%s%s%s%s%s", "\n$(OBJDIR)", target,
			".a: $(", target, "_OBJS)");
	if((p = _makefile_get_config(configure, target, "depends")) != NULL
			&& _makefile_expand(fp, p) != 0)
		return error_print(PROGNAME);
	_makefile_print(fp, "%c", '\n');
	/* build the static library */
	_makefile_print(fp, "%s%s%s%s%s",
			"\t$(AR) $(ARFLAGS) $(OBJDIR)", target, ".a $(",
			target, "_OBJS)");
	len = strlen(target) + 3;
	if((q = malloc(len)) == NULL)
		return 1;
	snprintf(q, len, "%s.a", target);
	if((p = _makefile_get_config(configure, q, "ldflags"))
			!= NULL)
		_binary_ldflags(configure, fp, p);
	free(q);
	_makefile_print(fp, "%s%s%s",
			"\n\t$(RANLIB) $(OBJDIR)", target, ".a\n");
	return 0;
}

static int _target_libtool(Configure * configure, FILE * fp,
		String const * target)
{
	String const * p;

	if(_target_objs(configure, fp, target) != 0)
		return 1;
	if(_target_flags(configure, fp, target) != 0)
		return 1;
	_makefile_print(fp, "%s%s%s%s%s", "\n$(OBJDIR)", target, ".la: $(",
			target, "_OBJS)\n");
	_makefile_print(fp, "%s%s%s%s%s",
			"\t$(LIBTOOL) --mode=link $(CC) -o $(OBJDIR)", target,
			".la $(", target, "_OBJS)");
	if((p = _makefile_get_config(configure, target, "ldflags")) != NULL)
		_binary_ldflags(configure, fp, p);
	_makefile_print(fp, "%s%s%s", " -rpath $(LIBDIR) $(", target,
			"_LDFLAGS)\n");
	return 0;
}

static int _target_object(Configure * configure, FILE * fp,
		String const * target)
{
	String const * p;
	String const * extension;

	if((p = _makefile_get_config(configure, target, "sources")) == NULL)
	{
		fprintf(stderr, "%s%s%s", PROGNAME ": ", target,
				": No sources for target\n");
		return 1;
	}
	if(strchr(p, ',') != NULL)
	{
		fprintf(stderr, "%s%s%s", PROGNAME ": ", target,
				": An object can have only one source file\n");
		return 1;
	}
	if((extension = _source_extension(p)) == NULL)
		return 1;
	switch(_source_type(extension))
	{
		case OT_ASM_SOURCE:
			_makefile_print(fp, "\n%s%s%s%s\n%s%s",
					target, "_OBJS = ", "$(OBJDIR)", target,
					target, "_ASFLAGS ="
					" $(CPPFLAGSF) $(CPPFLAGS) $(ASFLAGSF)"
					" $(ASFLAGS)");
			if((p = _makefile_get_config(configure, target,
							"asflags")) != NULL)
				_makefile_print(fp, " %s", p);
			_makefile_print(fp, "%c", '\n');
			break;
		case OT_C_SOURCE:
		case OT_OBJC_SOURCE:
			_makefile_print(fp, "\n%s%s%s%s\n%s%s",
					target, "_OBJS = ", "$(OBJDIR)", target,
					target, "_CFLAGS ="
					" $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF)"
					" $(CFLAGS)");
			if((p = _makefile_get_config(configure, target,
							"cflags")) != NULL)
				_makefile_print(fp, " %s", p);
			_makefile_print(fp, "%c", '\n');
			break;
		case OT_CXX_SOURCE:
		case OT_OBJCXX_SOURCE:
			_makefile_print(fp, "\n%s%s%s%s\n%s%s",
					target, "_OBJS = ", "$(OBJDIR)", target,
					target, "_CXXFLAGS ="
					" $(CPPFLAGSF) $(CPPFLAGS) $(CXXFLAGSF)"
					" $(CXXFLAGS)");
			if((p = _makefile_get_config(configure, target,
							"cxxflags")) != NULL)
				_makefile_print(fp, " %s", p);
			_makefile_print(fp, "%c", '\n');
			break;
		case OT_VERILOG_SOURCE:
			_makefile_print(fp, "\n%s%s%s%s\n%s%s",
					target, "_OBJS = ",
					"$(OBJDIR)", target, target, "_VFLAGS ="
					" $(VFLAGSF) $(VFLAGS)");
			if((p = _makefile_get_config(configure, target,
							"vflags")) != NULL)
				_makefile_print(fp, " %s", p);
			_makefile_print(fp, "%c", '\n');
			break;
		case OT_UNKNOWN:
			fprintf(stderr, "%s%s%s", PROGNAME ": ", target,
					": Unknown source type for object\n");
			return 1;
	}
	return 0;
}

static int _target_plugin(Configure * configure, FILE * fp,
		String const * target)
{
	String const * p;
	String * q;

	if(_target_objs(configure, fp, target) != 0)
		return 1;
	if(_target_flags(configure, fp, target) != 0)
		return 1;
	_makefile_print(fp, "%s%s%s%s%s", "\n$(OBJDIR)", target,
			"$(SOEXT): $(", target, "_OBJS)");
	if((p = _makefile_get_config(configure, target, "depends")) != NULL
			&& _makefile_expand(fp, p) != 0)
		return error_print(PROGNAME);
	_makefile_print(fp, "%c", '\n');
	/* build the plug-in */
	_makefile_print(fp, "%s%s%s%s%s%s%s", "\t$(CCSHARED) -o $(OBJDIR)",
			target, "$(SOEXT) $(", target, "_OBJS) $(", target,
			"_LDFLAGS)");
	if((q = string_new_append(target, "$(SOEXT)", NULL)) == NULL)
		return error_print(PROGNAME);
	if((p = _makefile_get_config(configure, q, "ldflags")) != NULL)
		_binary_ldflags(configure, fp, p);
	string_delete(q);
	_makefile_print(fp, "%c", '\n');
	return 0;
}

static int _objects_target(Configure * configure, FILE * fp,
		String const * target);
static int _write_objects(Configure * configure, FILE * fp)
{
	String const * p;
	String * targets;
	String * q;
	char c;
	size_t i;
	int ret = 0;

	if((p = _makefile_get_config(configure, NULL, "targets")) == NULL)
		return 0;
	if((targets = string_new(p)) == NULL)
		return 1;
	q = targets;
	for(i = 0;; i++)
	{
		if(targets[i] != ',' && targets[i] != '\0')
			continue;
		c = targets[i];
		targets[i] = '\0';
		ret += _objects_target(configure, fp, targets);
		if(c == '\0')
			break;
		targets += i + 1;
		i = 0;
	}
	string_delete(q);
	return ret;
}

static void _script_check(Configure * configure, String const * target,
		String const * script);
static int _script_depends(Configure * configure, FILE * fp,
		String const * target);
static String * _script_path(Configure * configure, String const * script);
static void _script_security(Configure * configure, String const * target,
		String const * script);
static int _target_script(Configure * configure, FILE * fp,
		String const * target)
{
	String const * prefix;
	String const * script;
	int phony;

	if((script = _makefile_get_config(configure, target, "script")) == NULL)
	{
		fprintf(stderr, "%s%s%s", PROGNAME ": ", target,
				": No script for target\n");
		return 1;
	}
	if(fp == NULL)
		_script_check(configure, target, script);
	if(_makefile_is_flag_set(configure, PREFS_S))
		_script_security(configure, target, script);
	phony = _makefile_is_phony(configure, target);
	_makefile_print(fp, "\n%s%s:", phony ? "" : "$(OBJDIR)",
			target);
	_script_depends(configure, fp, target);
	if((prefix = _makefile_get_config(configure, target, "prefix")) == NULL)
		prefix = "$(PREFIX)";
	_makefile_print(fp, "\n\t%s -P \"%s\" -- \"%s%s\"\n", script, prefix,
			phony ? "" : "$(OBJDIR)", target);
	return 0;
}

static void _script_check(Configure * configure, String const * target,
		String const * script)
{
	String * path;

	if((path = _script_path(configure, script)) == NULL)
	{
		error_print(PROGNAME);
		return;
	}
	/* XXX make it clear these are warnings */
	if(access(path, R_OK) != 0)
		error_set_print(PROGNAME, 0, "%s: %s%s%s%s%s", target, "The \"",
				path, "\" script is not readable (",
				strerror(errno), ")");
	else if(access(path, R_OK | X_OK) != 0)
		error_set_print(PROGNAME, 0, "%s: %s%s%s%s%s", target, "The \"",
				path, "\" script is not executable (",
				strerror(errno), ")");
	string_delete(path);
}

static int _script_depends(Configure * configure, FILE * fp,
		String const * target)
{
	String const * p;

	if((p = _makefile_get_config(configure, target, "depends")) != NULL
			&& _makefile_expand(fp, p) != 0)
		return error_print(PROGNAME);
	return 0;
}

static String * _script_path(Configure * configure, String const * script)
{
	String * ret;
	String const * directory;
	ssize_t i;
	String * p = NULL;

	if((directory = _makefile_get_config(configure, NULL, "directory"))
			== NULL)
	{
		error_print(PROGNAME);
		return NULL;
	}
	/* XXX truncate scripts at the first space (to allow arguments) */
	if((i = string_index(script, " ")) > 0)
	{
		if((p = string_new_length(script, i)) == NULL)
		{
			error_print(PROGNAME);
			return NULL;
		}
		script = p;
	}
	if(script[0] == '/')
		ret = string_new(script);
	else if(string_compare_length(script, "./", 2) == 0)
		ret = string_new_append(directory, &script[1], NULL);
	else
		ret = string_new_append(directory, "/", script, NULL);
	string_delete(p);
	return ret;
}

static void _script_security(Configure * configure, String const * target,
		String const * script)
{
	String * path;

	if((path = _script_path(configure, script)) == NULL)
	{
		error_print(PROGNAME);
		return;
	}
	error_set_print(PROGNAME, 0, "%s: %s%s%s", target, "The \"", path,
			"\" script is executed while compiling");
	string_delete(path);
}

static int _target_source(Configure * configure, FILE * fp,
		String const * target, String * source);
static int _objects_target(Configure * configure, FILE * fp,
		String const * target)
{
	int ret = 0;
	String const * p;
	String * sources;
	String * q;
	size_t i;
	char c;

	if((p = _makefile_get_config(configure, target, "sources")) == NULL)
		return 0;
	if((sources = string_new(p)) == NULL)
		return 1;
	q = sources;
	for(i = 0;; i++)
	{
		if(sources[i] != ',' && sources[i] != '\0')
			continue;
		c = sources[i];
		sources[i] = '\0';
		ret |= _target_source(configure, fp, target, sources);
		if(c == '\0')
			break;
		sources += i + 1;
		i = 0;
	}
	string_delete(q);
	return ret;
}

static int _source_depends(Configure * configure, FILE * fp,
		String const * source);
static int _source_subdir(FILE * fp, String * source);
static int _target_source(Configure * configure, FILE * fp,
		String const * target, String * source)
	/* FIXME check calls to _source_depends() */
{
	int ret = 0;
	String const * extension;
	TargetType tt = TT_UNKNOWN;
	ObjectType ot;
	size_t len;
	String const * p;
	String const * q;

	if((p = _makefile_get_config(configure, target, "type")) != NULL)
			tt = enum_string(TT_LAST, sTargetType, p);
	if((extension = _source_extension(source)) == NULL)
		return 1;
	len = string_length(source) - string_length(extension) - 1;
	source[len] = '\0';
	switch((ot = _source_type(extension)))
	{
		case OT_ASM_SOURCE:
			if(tt == TT_OBJECT)
				_makefile_print(fp, "%s%s", "\n$(OBJDIR)",
						target);
			else
				_makefile_print(fp, "%s%s%s",
						"\n$(OBJDIR)", source, ".o");
			if(tt == TT_LIBTOOL)
				_makefile_print(fp, " %s.lo", source);
			_makefile_print(fp, "%s%s%s%s", ": ", source, ".",
					extension);
			source[len] = '.'; /* FIXME ugly */
			_source_depends(configure, fp, source);
			source[len] = '\0';
			_makefile_print(fp, "%s", "\n\t");
			if(strchr(source, '/') != NULL)
				ret = _source_subdir(fp, source);
			if(tt == TT_LIBTOOL)
				_makefile_print(fp, "%s",
						"$(LIBTOOL) --mode=compile ");
			_makefile_print(fp, "%s%s%s", "$(AS) $(", target,
					"_ASFLAGS)");
			if(tt == TT_OBJECT)
				_makefile_print(fp, "%s%s%s%s%s%s",
						" -o $(OBJDIR)", target, " ",
						source, ".", extension);
			else
				_makefile_print(fp, "%s%s%s%s%s%s",
						" -o $(OBJDIR)", source, ".o ",
						source, ".", extension);
			_makefile_print(fp, "%c", '\n');
			break;
		case OT_C_SOURCE:
		case OT_OBJC_SOURCE:
			if(tt == TT_OBJECT)
				_makefile_print(fp, "%s%s", "\n$(OBJDIR)",
						target);
			else
				_makefile_print(fp, "%s%s%s", "\n$(OBJDIR)",
						source, ".o");
			if(tt == TT_LIBTOOL)
				_makefile_print(fp, " %s%s", source, ".lo");
			_makefile_print(fp, "%s%s%s%s", ": ", source, ".",
					extension);
			source[len] = '.'; /* FIXME ugly */
			_source_depends(configure, fp, source);
			_makefile_print(fp, "%s", "\n\t");
			if(strchr(source, '/') != NULL)
				ret = _source_subdir(fp, source);
			/* FIXME do both wherever also relevant */
			p = _makefile_get_config(configure, source, "cppflags");
			q = _makefile_get_config(configure, source, "cflags");
			source[len] = '\0';
			if(tt == TT_LIBTOOL)
				_makefile_print(fp, "%s",
						"$(LIBTOOL) --mode=compile ");
			_makefile_print(fp, "%s", "$(CC)");
			if(p != NULL)
				_makefile_print(fp, " %s", p);
			_makefile_print(fp, "%s%s%s", " $(", target,
					"_CFLAGS)");
			if(q != NULL)
			{
				_makefile_print(fp, " %s", q);
				if(configure_get_os(configure) == HO_GNU_LINUX
					       	&& string_find(q, "-ansi"))
					_makefile_print(fp, "%s",
							" -D _GNU_SOURCE");
			}
			if(tt == TT_OBJECT)
				_makefile_print(fp, "%s%s",
						" -o $(OBJDIR)", target);
			else
				_makefile_print(fp, "%s%s%s",
						" -o $(OBJDIR)", source, ".o");
			_makefile_print(fp, "%s%s%s%s%c", " -c ", source, ".",
					extension, '\n');
			break;
		case OT_CXX_SOURCE:
		case OT_OBJCXX_SOURCE:
			if(tt == TT_OBJECT)
				_makefile_print(fp, "%s%s", "\n$(OBJDIR)",
						target);
			else
				_makefile_print(fp, "%s%s%s", "\n$(OBJDIR)",
						source, ".o");
			_makefile_print(fp, "%s%s%s%s", ": ", source, ".",
					extension);
			source[len] = '.'; /* FIXME ugly */
			_source_depends(configure, fp, source);
			p = _makefile_get_config(configure, source, "cxxflags");
			source[len] = '\0';
			_makefile_print(fp, "%s", "\n\t");
			if(strchr(source, '/') != NULL)
				ret = _source_subdir(fp, source);
			_makefile_print(fp, "%s%s%s", "$(CXX) $(", target,
					"_CXXFLAGS)");
			if(p != NULL)
				_makefile_print(fp, " %s", p);
			if(tt == TT_OBJECT)
				_makefile_print(fp, "%s%s", " -o $(OBJDIR)",
						target);
			else
				_makefile_print(fp, "%s%s%s", " -o $(OBJDIR)",
						source, ".o");
			_makefile_print(fp, "%s%s%s%s\n", " -c ", source, ".",
					extension);
			break;
		case OT_VERILOG_SOURCE:
			if(tt == TT_OBJECT)
				_makefile_print(fp, "%s%s", "\n$(OBJDIR)",
						target);
			else
				_makefile_print(fp, "%s%s%s", "\n$(OBJDIR)",
						source, ".o");
			_makefile_print(fp, "%s%s%s%s", ": ", source, ".",
					extension);
			source[len] = '.'; /* FIXME ugly */
			_source_depends(configure, fp, source);
			_makefile_print(fp, "%s", "\n\t");
			if(strchr(source, '/') != NULL)
				ret = _source_subdir(fp, source);
			q = _makefile_get_config(configure, source, "vflags");
			source[len] = '\0';
			_makefile_print(fp, "%s", "$(VERILOG)");
			_makefile_print(fp, "%s%s%s", " $(", target,
					"_VFLAGS)");
			if(q != NULL)
				_makefile_print(fp, " %s", q);
			if(tt == TT_OBJECT)
				_makefile_print(fp, "%s%s",
						" -o $(OBJDIR)", target);
			else
				_makefile_print(fp, "%s%s%s",
						" -o $(OBJDIR)", source, ".o");
			_makefile_print(fp, "%s%s%s%s%c", " ", source, ".",
					extension, '\n');
			break;
		case OT_UNKNOWN:
			fprintf(stderr, "%s%s%s", PROGNAME ": ", target,
					": Unknown source type for object\n");
			ret = 1;
			break;
	}
	source[len] = '.';
	return ret;
}

static int _source_depends(Configure * configure, FILE * fp,
		String const * target)
{
	String const * p;

	if((p = _makefile_get_config(configure, target, "depends")) != NULL
			&& _makefile_expand(fp, p) != 0)
		return error_print(PROGNAME);
	return 0;
}

static int _source_subdir(FILE * fp, String * source)
{
	char * p;
	char const * q;

	if((p = strdup(source)) == NULL)
		return 1;
	q = dirname(p);
	_makefile_print(fp, "@[ -d \"%s%s\" ] || $(MKDIR) -- \"%s%s\"\n\t",
			"$(OBJDIR)", q, "$(OBJDIR)", q);
	free(p);
	return 0;
}

static int _clean_targets(Configure * configure, FILE * fp);
static int _write_clean(Configure * configure, FILE * fp)
{
	_makefile_target(fp, "clean", NULL);
	if(_makefile_get_config(configure, NULL, "subdirs") != NULL)
		_makefile_subdirs(fp, "clean");
	return _clean_targets(configure, fp);
}

static int _clean_targets(Configure * configure, FILE * fp)
{
	String const * prefix;
	String const * p;
	String * targets;
	String * q;
	size_t i;
	char c;

	if((p = _makefile_get_config(configure, NULL, "targets")) == NULL)
		return 0;
	if((targets = string_new(p)) == NULL)
		return 1;
	q = targets;
	/* remove all of the object files */
	_makefile_print(fp, "%s", "\t$(RM) --");
	for(i = 0;; i++)
	{
		if(targets[i] != ',' && targets[i] != '\0')
			continue;
		c = targets[i];
		targets[i] = '\0';
		_makefile_print(fp, "%s%s%s", " $(", targets, "_OBJS)");
		if(c == '\0')
			break;
		targets[i] = c;
		targets += i + 1;
		i = 0;
	}
	_makefile_print(fp, "%c", '\n');
	targets = q;
	/* let each scripted target remove the relevant object files */
	for(i = 0;; i++)
	{
		if(targets[i] != ',' && targets[i] != '\0')
			continue;
		c = targets[i];
		targets[i] = '\0';
		if((p = _makefile_get_config(configure, targets, "type")) != NULL
				&& strcmp(p, "script") == 0
				&& (p = _makefile_get_config(configure, targets,
						"script")) != NULL)
		{
			if((prefix = _makefile_get_config(configure, targets,
							"prefix")) == NULL)
				prefix = "$(PREFIX)";
			_makefile_print(fp, "\t%s%s%s%s%s%s\n", p, " -c -P \"",
					prefix, "\" -- \"$(OBJDIR)", targets,
					"\"");
		}
		if(c == '\0')
			break;
		targets[i] = c;
		targets += i + 1;
		i = 0;
	}
	string_delete(q);
	return 0;
}

static int _write_distclean(Configure * configure, FILE * fp)
{
	String const * subdirs;

	/* only depend on the "clean" target if we do not have subfolders */
	if((subdirs = _makefile_get_config(configure, NULL, "subdirs")) == NULL)
		_makefile_target(fp, "distclean", "clean", NULL);
	else
	{
		_makefile_target(fp, "distclean", NULL);
		_makefile_subdirs(fp, "distclean");
		_clean_targets(configure, fp);
	}
	/* FIXME do not erase targets that need be distributed */
	if(_makefile_get_config(configure, NULL, "targets") != NULL)
		_makefile_remove(fp, 0, "$(TARGETS)", NULL);
	return 0;
}

static int _dist_subdir(Configure * configure, FILE * fp, Config * subdir);
static int _write_dist(Configure * configure, FILE * fp, configArray * ca,
	       	int from, int to)
{
	String const * package;
	String const * version;
	Config * p;
	int i;

	package = _makefile_get_config(configure, NULL, "package");
	version = _makefile_get_config(configure, NULL, "version");
	if(package == NULL || version == NULL)
		return 0;
	_makefile_target(fp, "dist", NULL);
	_makefile_remove(fp, 1, "$(OBJDIR)$(PACKAGE)-$(VERSION)", NULL);
	_makefile_link(fp, 1, "\"$$PWD\"", "$(OBJDIR)$(PACKAGE)-$(VERSION)");
	_makefile_print(fp, "%s", "\t@cd $(OBJDIR). && $(TAR) -czvf"
			" $(PACKAGE)-$(VERSION).tar.gz -- \\\n");
	for(i = from + 1; i < to; i++)
	{
		array_get_copy(ca, i, &p);
		_dist_subdir(configure, fp, p);
	}
	if(from < to)
	{
		array_get_copy(ca, from, &p);
		_dist_subdir(configure, fp, p);
	}
	else
		return 1;
	_makefile_remove(fp, 0, "$(OBJDIR)$(PACKAGE)-$(VERSION)", NULL);
	return 0;
}

static int _write_distcheck(Configure * configure, FILE * fp)
{
	String const * package;
	String const * version;
	const char pretarget[] = "\ndistcheck: dist\n"
		"\t$(TAR) -xzvf $(OBJDIR)$(PACKAGE)-$(VERSION).tar.gz\n"
		"\t$(MKDIR) -- $(PACKAGE)-$(VERSION)/objdir\n"
		"\t$(MKDIR) -- $(PACKAGE)-$(VERSION)/destdir\n";
	const char target[] = "\tcd \"$(PACKAGE)-$(VERSION)\" && $(MAKE) OBJDIR=\"$$PWD/objdir/\"\n"
		"\tcd \"$(PACKAGE)-$(VERSION)\" && $(MAKE) OBJDIR=\"$$PWD/objdir/\" DESTDIR=\"$$PWD/destdir\" install\n"
		"\tcd \"$(PACKAGE)-$(VERSION)\" && $(MAKE) OBJDIR=\"$$PWD/objdir/\" DESTDIR=\"$$PWD/destdir\" uninstall\n"
		"\tcd \"$(PACKAGE)-$(VERSION)\" && $(MAKE) OBJDIR=\"$$PWD/objdir/\" distclean\n"
		"\tcd \"$(PACKAGE)-$(VERSION)\" && $(MAKE) dist\n";
	const char posttarget[] = "\t$(RM) -r -- $(PACKAGE)-$(VERSION)\n";

	package = _makefile_get_config(configure, NULL, "package");
	version = _makefile_get_config(configure, NULL, "version");
	if(package == NULL || version == NULL)
		return 0;
	_makefile_print(fp, "%s%s%s", pretarget, target, posttarget);
	return 0;
}

static int _dist_subdir_dist(FILE * fp, String const * path,
		String const * dist);
static int _dist_subdir(Configure * configure, FILE * fp, Config * subdir)
{
	String const * path;
	size_t len;
	String const * p;
	String * targets;
	String * q;
	String const * includes;
	String const * dist;
	size_t i;
	char c;
	String const * quote;

	path = _makefile_get_config(configure, NULL, "directory");
	len = string_length(path);
	path = config_get(subdir, NULL, "directory");
	path = &path[len];
	if(path[0] == '/')
		path++;
	if((p = config_get(subdir, NULL, "targets")) != NULL)
	{
		/* FIXME unique SOURCES */
		if((targets = string_new(p)) == NULL)
			return 1;
		q = targets;
		for(i = 0;; i++)
		{
			if(targets[i] != ',' && targets[i] != '\0')
				continue;
			c = targets[i];
			targets[i] = '\0';
			if((dist = config_get(subdir, targets, "sources"))
					!= NULL)
				_dist_subdir_dist(fp, path, dist);
			if(c == '\0')
				break;
			targets += i + 1;
			i = 0;
		}
		string_delete(q);
	}
	if((includes = config_get(subdir, NULL, "includes")) != NULL)
		_dist_subdir_dist(fp, path, includes);
	if((dist = config_get(subdir, NULL, "dist")) != NULL)
		_dist_subdir_dist(fp, path, dist);
	quote = (strchr(path, ' ') != NULL) ? "\"" : "";
	_makefile_print(fp, "%s%s%s%s%s%s%s%s", "\t\t", quote,
			"$(PACKAGE)-$(VERSION)/", path,
			(path[0] == '\0') ? "" : "/", PROJECT_CONF, quote,
			(path[0] == '\0') ? "\n" : " \\\n");
	return 0;
}

static int _dist_subdir_dist(FILE * fp, String const * path,
		String const * dist)
{
	String * d;
	String * p;
	size_t i;
	char c;
	String const * quote;

	if((d = string_new(dist)) == NULL)
		return 1;
	p = d;
	for(i = 0;; i++)
	{
		if(d[i] != ',' && d[i] != '\0')
			continue;
		c = d[i];
		d[i] = '\0';
		quote = (strchr(path, ' ') != NULL || strchr(d, ' ') != NULL)
			? "\"" : "";
		_makefile_print(fp, "%s%s%s%s%s%s%s%s", "\t\t", quote,
				"$(PACKAGE)-$(VERSION)/",
				(path[0] == '\0') ? "" : path,
				(path[0] == '\0') ? "" : "/",
				d, quote, " \\\n");
		if(c == '\0')
			break;
		d[i] = c;
		d += i + 1;
		i = 0;
	}
	string_delete(p);
	return 0;
}

static int _install_targets(Configure * configure, FILE * fp);
static int _install_includes(Configure * configure, FILE * fp);
static int _install_dist(Configure * configure, FILE * fp);
static int _write_install(Configure * configure, FILE * fp)
{
	int ret = 0;
	char const * targets;

	targets = _makefile_get_config(configure, NULL, "targets");
	_makefile_target(fp, "install", (targets != NULL) ? "$(TARGETS)" : NULL,
			NULL);
	if(_makefile_get_config(configure, NULL, "subdirs") != NULL)
		_makefile_subdirs(fp, "install");
	ret |= _install_targets(configure, fp);
	ret |= _install_includes(configure, fp);
	ret |= _install_dist(configure, fp);
	return ret;
}

static int _install_target(Configure * configure, FILE * fp,
		String const * target);
static int _install_targets(Configure * configure, FILE * fp)
{
	int ret = 0;
	String const * p;
	String * q;
	String * targets;
	size_t i;
	char c;

	if((p = _makefile_get_config(configure, NULL, "targets")) == NULL)
		return 0;
	if((targets = string_new(p)) == NULL)
		return 1;
	q = targets;
	for(i = 0; ret == 0; i++)
	{
		if(targets[i] != ',' && targets[i] != '\0')
			continue;
		c = targets[i];
		targets[i] = '\0';
		ret |= _install_target(configure, fp, targets);
		if(c == '\0')
			break;
		targets += i + 1;
		i = 0;
	}
	string_delete(q);
	return ret;
}

static void _install_target_binary(Configure * configure, FILE * fp,
		String const * target);
static int _install_target_library(Configure * configure, FILE * fp,
		String const * target);
static void _install_target_libtool(Configure * configure, FILE * fp,
		String const * target);
static void _install_target_object(Configure * configure, FILE * fp,
		String const * target);
static void _install_target_plugin(Configure * configure, FILE * fp,
		String const * target);
static void _install_target_script(Configure * configure, FILE * fp,
		String const * target);
static int _install_target(Configure * configure, FILE * fp,
		String const * target)
{
	int ret = 0;
	String const * type;
	TargetType tt;

	if((type = _makefile_get_config(configure, target, "type")) == NULL)
		return 1;
	switch((tt = enum_string(TT_LAST, sTargetType, type)))
	{
		case TT_BINARY:
			_install_target_binary(configure, fp, target);
			break;
		case TT_LIBRARY:
			ret = _install_target_library(configure, fp, target);
			break;
		case TT_LIBTOOL:
			_install_target_libtool(configure, fp, target);
			break;
		case TT_OBJECT:
			_install_target_object(configure, fp, target);
			break;
		case TT_PLUGIN:
			_install_target_plugin(configure, fp, target);
			break;
		case TT_SCRIPT:
			_install_target_script(configure, fp, target);
			break;
		case TT_UNKNOWN:
			break;
	}
	return ret;
}

static void _install_target_binary(Configure * configure, FILE * fp,
		String const * target)
{
	String const * path;

	if((path = _makefile_get_config(configure, target, "install")) == NULL)
		return;
	_makefile_mkdir(fp, path);
	_makefile_print(fp, "%s%s%s%s/%s%s\n",
			"\t$(INSTALL) -m 0755 $(OBJDIR)", target,
			"$(EXEEXT) $(DESTDIR)", path, target, "$(EXEEXT)");
}

static int _install_target_library(Configure * configure, FILE * fp,
		String const * target)
{
	String const * path;
	String const * p;
	String * soname;

	if((path = _makefile_get_config(configure, target, "install")) == NULL)
		return 0;
	_makefile_mkdir(fp, path);
	if(configure_can_library_static(configure))
		/* install the static library */
		_makefile_print(fp, "%s%s%s%s/%s%s",
				"\t$(INSTALL) -m 0644 $(OBJDIR)", target,
				".a $(DESTDIR)", path, target, ".a\n");
	if((p = _makefile_get_config(configure, target, "soname")) != NULL)
		soname = string_new(p);
	else if(configure_get_os(configure) == HO_MACOSX)
		/* versioning is different on MacOS X */
		soname = string_new_append(target, ".0.0$(SOEXT)", NULL);
	else if(configure_get_os(configure) == HO_WIN32)
		/* and on Windows */
		soname = string_new_append(target, "$(SOEXT)", NULL);
	else
		soname = string_new_append(target, "$(SOEXT)", ".0", NULL);
	if(soname == NULL)
		return 1;
	/* install the shared library */
	if(configure_get_os(configure) == HO_MACOSX)
	{
		_makefile_print(fp, "%s%s%s%s/%s%s", "\t$(INSTALL) -m 0755 $(OBJDIR)",
				soname, " $(DESTDIR)", path, soname, "\n");
		_makefile_print(fp, "%s%s%s%s/%s%s", "\t$(LN) -s -- ", soname,
				" $(DESTDIR)", path, target, ".0$(SOEXT)\n");
		_makefile_print(fp, "%s%s%s%s/%s%s", "\t$(LN) -s -- ", soname,
				" $(DESTDIR)", path, target, "$(SOEXT)\n");
	}
	else if(configure_get_os(configure) == HO_WIN32)
		_makefile_print(fp, "%s%s%s%s%s%s%s", "\t$(INSTALL) -m 0755 $(OBJDIR)",
				soname, " $(DESTDIR)", path, "/", soname, "\n");
	else
	{
		_makefile_print(fp, "%s%s%s%s/%s%s", "\t$(INSTALL) -m 0755 $(OBJDIR)",
				soname, ".0 $(DESTDIR)", path, soname, ".0\n");
		_makefile_print(fp, "%s%s%s%s/%s%s", "\t$(LN) -s -- ", soname,
				".0 $(DESTDIR)", path, soname, "\n");
		_makefile_print(fp, "%s%s%s%s/%s%s", "\t$(LN) -s -- ", soname,
				".0 $(DESTDIR)", path, target, "$(SOEXT)\n");
	}
	string_delete(soname);
	return 0;
}

static void _install_target_libtool(Configure * configure, FILE * fp,
		String const * target)
{
	String const * path;

	if((path = _makefile_get_config(configure, target, "install")) == NULL)
		return;
	_makefile_mkdir(fp, path);
	_makefile_print(fp, "%s%s%s%s/%s%s",
			"\t$(LIBTOOL) --mode=install $(INSTALL)"
			" -m 0755 $(OBJDIR)", target, ".la $(DESTDIR)", path,
			target, ".la\n");
	_makefile_print(fp, "%s/%s\n", "\t$(LIBTOOL) --mode=finish $(DESTDIR)",
			path);
}

static void _install_target_object(Configure * configure, FILE * fp,
		String const * target)
{
	String const * path;

	if((path = _makefile_get_config(configure, target, "install")) == NULL)
		return;
	_makefile_mkdir(fp, path);
	_makefile_print(fp, "%s%s%s%s/%s\n", "\t$(INSTALL) -m 0644 $(OBJDIR)",
			target, " $(DESTDIR)", path, target);
}

static void _install_target_plugin(Configure * configure, FILE * fp,
		String const * target)
{
	String const * path;
	String const * mode;
	mode_t m = 0755;
	String * p;

	if((path = _makefile_get_config(configure, target, "install")) == NULL)
		return;
	if((mode = _makefile_get_config(configure, target, "mode")) == NULL
			/* XXX these tests are not sufficient */
			|| mode[0] == '\0'
			|| (m = strtol(mode, &p, 8)) == 0
			|| *p != '\0')
		mode = "0755";
	_makefile_mkdir(fp, path);
	_makefile_print(fp, "%s%04o%s%s%s%s%s%s%s", "\t$(INSTALL) -m ", m,
			" $(OBJDIR)", target, "$(SOEXT) $(DESTDIR)", path, "/",
			target, "$(SOEXT)\n");
}

static void _install_target_script(Configure * configure, FILE * fp,
		String const * target)
{
	String const * path;
	String const * script;
	int phony;

	if((path = _makefile_get_config(configure, target, "install")) == NULL)
		return;
	if((script = _makefile_get_config(configure, target, "script")) == NULL)
		return;
	phony = _makefile_is_phony(configure, target);
	_makefile_print(fp, "\t%s%s%s%s%s%s%s", script, " -P \"$(DESTDIR)",
			(path[0] != '\0') ? path : "$(PREFIX)",
			"\" -i -- \"", phony ? "" : "$(OBJDIR)", target,
			"\"\n");
}

static int _install_include(Configure * configure, FILE * fp,
		String const * include);
static int _install_includes(Configure * configure, FILE * fp)
{
	int ret = 0;
	String const * p;
	String * includes;
	String * q;
	size_t i;
	char c;

	if((p = _makefile_get_config(configure, NULL, "includes")) == NULL)
		return 0;
	if((includes = string_new(p)) == NULL)
		return 1;
	q = includes;
	for(i = 0; ret == 0; i++)
	{
		if(includes[i] != ',' && includes[i] != '\0')
			continue;
		c = includes[i];
		includes[i] = '\0';
		ret |= _install_include(configure, fp, includes);
		if(c == '\0')
			break;
		includes += i + 1;
		i = 0;
	}
	string_delete(q);
	return ret;
}

static int _install_include(Configure * configure, FILE * fp,
		String const * include)
{
	char const * install;
	ssize_t i;
	String * p = NULL;

	if((install = _makefile_get_config(configure, include, "install"))
			== NULL)
	{
		install = "$(INCLUDEDIR)";
		if((i = string_rindex(include, "/")) >= 0)
			if((p = string_new_length(include, i)) == NULL)
				return 2;
	}
	/* FIXME keep track of the directories created */
	_makefile_print(fp, "%s%s", "\t$(MKDIR) $(DESTDIR)", install);
	if(p != NULL)
	{
		_makefile_print(fp, "/%s", p);
		string_delete(p);
	}
	_makefile_print(fp, "\n");
	_makefile_print(fp, "%s%s%s%s/%s\n", "\t$(INSTALL) -m 0644 ", include,
			" $(DESTDIR)", install, include);
	return 0;
}

static int _dist_check(Configure * configure, char const * target,
		char const * mode);
static int _dist_install(Configure * configure, FILE * fp,
		char const * directory, char const * mode,
		char const * filename);
static int _install_dist(Configure * configure, FILE * fp)
{
	int ret = 0;
	String const * p;
	String * dist;
	String * q;
	size_t i;
	char c;
	String const * d;
	String const * mode;

	if((p = _makefile_get_config(configure, NULL, "dist")) == NULL)
		return 0;
	if((dist = string_new(p)) == NULL)
		return 1;
	q = dist;
	for(i = 0;; i++)
	{
		if(dist[i] != ',' && dist[i] != '\0')
			continue;
		c = dist[i];
		dist[i] = '\0';
		if((mode = _makefile_get_config(configure, dist, "mode"))
				== NULL)
			mode = "0644";
		ret |= _dist_check(configure, dist, mode);
		if((d = _makefile_get_config(configure, dist, "install"))
				!= NULL)
			_dist_install(configure, fp, d, mode, dist);
		if(c == '\0')
			break;
		dist += i + 1;
		i = 0;
	}
	string_delete(q);
	return ret;
}

static int _dist_check(Configure * configure, char const * target,
		char const * mode)
{
	char * p;
	mode_t m;

	m = strtol(mode, &p, 8);
	if(mode[0] == '\0' || *p != '\0')
		return error_set_print(PROGNAME, 1, "%s: %s%s%s", target,
				"Invalid permissions \"", mode, "\"");
	if(_makefile_is_flag_set(configure, PREFS_S) && (m & S_ISUID))
		error_set_print(PROGNAME, 0, "%s: %s", target,
				"Installed as a SUID file");
	if(_makefile_is_flag_set(configure, PREFS_S) && (m & S_ISUID))
		error_set_print(PROGNAME, 0, "%s: %s", target,
				"Installed as a SGID file");
	if(_makefile_is_flag_set(configure, PREFS_S)
			&& (m & (S_IXUSR | S_IXGRP | S_IXOTH)))
		error_set_print(PROGNAME, 0, "%s: %s", target,
				"Installed as an executable file");
	if(_makefile_is_flag_set(configure, PREFS_S) && (m & S_IWGRP))
		error_set_print(PROGNAME, 0, "%s: %s", target,
				"Installed as a group-writable file");
	if(_makefile_is_flag_set(configure, PREFS_S) && (m & S_IWOTH))
		error_set_print(PROGNAME, 0, "%s: %s", target,
				"Installed as a writable file");
	return 0;
}

static int _dist_install(Configure * configure, FILE * fp,
		char const * directory, char const * mode,
		char const * filename)
{
	char sep = (configure_get_os(configure) != HO_WIN32) ? '/' : '\\';
	String * p;
	char const * q;

	if(strchr(filename, sep) != NULL)
	{
		if((p = string_new(filename)) == NULL)
			return -1;
		q = dirname(p);
		/* FIXME keep track of the directories created */
		_makefile_print(fp, "%s%s%c%s\n", "\t$(MKDIR) $(DESTDIR)",
				directory, sep, q);
		string_delete(p);
	}
	else
		_makefile_mkdir(fp, directory);
	_makefile_print(fp, "%s%s%s%s%s%s%c%s\n", "\t$(INSTALL) -m ", mode, " ",
			filename, " $(DESTDIR)", directory, sep, filename);
	return 0;
}

static int _write_phony_targets(Configure * configure, FILE * fp);
static int _write_phony(Configure * configure, FILE * fp, char const ** targets)
{
	size_t i;

	_makefile_print(fp, "%s:", "\n.PHONY");
	for(i = 0; targets[i] != NULL; i++)
		_makefile_print(fp, " %s", targets[i]);
	if(_write_phony_targets(configure, fp) != 0)
		return 1;
	_makefile_print(fp, "%s", "\n");
	return 0;
}

static int _write_phony_targets(Configure * configure, FILE * fp)
{
	String const * p;
	String * prints;
	size_t i;
	char c;
	String const * type;

	if((p = _makefile_get_config(configure, NULL, "targets")) == NULL)
		return 0;
	if((prints = string_new(p)) == NULL)
		return 1;
	for(i = 0;; i++)
	{
		if(prints[i] != ',' && prints[i] != '\0')
			continue;
		c = prints[i];
		prints[i] = '\0';
		if((type = _makefile_get_config(configure, prints, "type"))
				!= NULL)
			switch(enum_string(TT_LAST, sTargetType, type))
			{
				case TT_SCRIPT:
					if(_makefile_is_phony(configure,
								prints))
						_makefile_print(fp, " %s",
								prints);
					break;
			}
		if(c == '\0')
			break;
		prints += i + 1;
		i = 0;
	}
	return 0;
}

static int _uninstall_target(Configure * configure, FILE * fp,
		String const * target);
static int _uninstall_include(Configure * configure, FILE * fp,
		String const * include);
static int _uninstall_dist(Configure * configure, FILE * fp,
		String const * dist);
static int _write_uninstall(Configure * configure, FILE * fp)
{
	int ret = 0;
	String const * p;
	String * targets;
	String * q;
	String * includes;
	String * dist;
	size_t i;
	char c;

	_makefile_target(fp, "uninstall", NULL);
	if(_makefile_get_config(configure, NULL, "subdirs") != NULL)
		_makefile_subdirs(fp, "uninstall");
	if((p = _makefile_get_config(configure, NULL, "targets")) != NULL)
	{
		if((targets = string_new(p)) == NULL)
			return 1;
		q = targets;
		for(i = 0; ret == 0; i++)
		{
			if(targets[i] != ',' && targets[i] != '\0')
				continue;
			c = targets[i];
			targets[i] = '\0';
			ret = _uninstall_target(configure, fp, targets);
			if(c == '\0')
				break;
			targets += i + 1;
			i = 0;
		}
		string_delete(q);
	}
	if((p = _makefile_get_config(configure, NULL, "includes")) != NULL)
	{
		if((includes = string_new(p)) == NULL)
			return 1;
		q = includes;
		for(i = 0; ret == 0; i++)
		{
			if(includes[i] != ',' && includes[i] != '\0')
				continue;
			c = includes[i];
			includes[i] = '\0';
			ret = _uninstall_include(configure, fp, includes);
			if(c == '\0')
				break;
			includes += i + 1;
			i = 0;
		}
		string_delete(q);
	}
	if((p = _makefile_get_config(configure, NULL, "dist")) != NULL)
	{
		if((dist = string_new(p)) == NULL)
			return 1;
		q = dist;
		for(i = 0; ret == 0; i++)
		{
			if(dist[i] != ',' && dist[i] != '\0')
				continue;
			c = dist[i];
			dist[i] = '\0';
			ret = _uninstall_dist(configure, fp, dist);
			if(c == '\0')
				break;
			dist += i + 1;
			i = 0;
		}
		string_delete(q);
	}
	return ret;
}

static int _uninstall_target_library(Configure * configure, FILE * fp,
		String const * target, String const * path);
static void _uninstall_target_script(Configure * configure, FILE * fp,
		String const * target, String const * path);
static int _uninstall_target(Configure * configure, FILE * fp,
		String const * target)
{
	String const * type;
	String const * path;
	TargetType tt;
	const String rm_destdir[] = "$(RM) -- $(DESTDIR)";

	if((type = _makefile_get_config(configure, target, "type")) == NULL)
		return 1;
	if((path = _makefile_get_config(configure, target, "install")) == NULL)
		return 0;
	tt = enum_string(TT_LAST, sTargetType, type);
	switch(tt)
	{
		case TT_BINARY:
			_makefile_print(fp, "\t%s%s/%s%s\n", rm_destdir, path,
					target, "$(EXEEXT)");
			break;
		case TT_LIBRARY:
			if(_uninstall_target_library(configure, fp, target,
						path) != 0)
				return 1;
			break;
		case TT_LIBTOOL:
			_makefile_print(fp, "\t%s%s%s/%s%s", "$(LIBTOOL)"
					" --mode=uninstall ", rm_destdir, path,
					target, ".la\n");
			break;
		case TT_OBJECT:
			_makefile_print(fp, "\t%s%s/%s\n", rm_destdir, path,
					target);
			break;
		case TT_PLUGIN:
			_makefile_print(fp, "\t%s%s/%s%s\n", rm_destdir, path,
					target, "$(SOEXT)");
			break;
		case TT_SCRIPT:
			_uninstall_target_script(configure, fp, target, path);
			break;
		case TT_UNKNOWN:
			break;
	}
	return 0;
}

static int _uninstall_target_library(Configure * configure, FILE * fp,
		String const * target, String const * path)
{
	String * soname;
	String const * p;
	const String format[] = "\t%s%s/%s%s%s%s";
	const String rm_destdir[] = "$(RM) -- $(DESTDIR)";

	if(configure_can_library_static(configure))
		/* uninstall the static library */
		_makefile_print(fp, format, rm_destdir, path, target, ".a\n",
				"", "");
	if((p = _makefile_get_config(configure, target, "soname")) != NULL)
		soname = string_new(p);
	else if(configure_get_os(configure) == HO_MACOSX)
		/* versioning is different on MacOS X */
		soname = string_new_append(target, ".0.0$(SOEXT)", NULL);
	else if(configure_get_os(configure) == HO_WIN32)
		/* and on Windows */
		soname = string_new_append(target, "$(SOEXT)", NULL);
	else
		soname = string_new_append(target, "$(SOEXT).0", NULL);
	if(soname == NULL)
		return 1;
	/* uninstall the shared library */
	if(configure_get_os(configure) == HO_MACOSX)
	{
		_makefile_print(fp, format, rm_destdir, path, soname, "\n", "",
				"");
		_makefile_print(fp, format, rm_destdir, path, target, ".0",
				"$(SOEXT)", "\n");
	}
	else if(configure_get_os(configure) != HO_WIN32)
	{
		_makefile_print(fp, format, rm_destdir, path, soname, ".0\n",
				"", "");
		_makefile_print(fp, format, rm_destdir, path, soname, "\n", "",
				"");
	}
	_makefile_print(fp, format, rm_destdir, path, target, "$(SOEXT)", "\n",
			"");
	string_delete(soname);
	return 0;
}

static void _uninstall_target_script(Configure * configure, FILE * fp,
		String const * target, String const * path)
{
	String const * script;

	if((script = _makefile_get_config(configure, target, "script")) == NULL)
		return;
	_makefile_print(fp, "\t%s%s%s%s%s%s", script, " -P \"$(DESTDIR)",
			(path[0] != '\0') ? path : "$(PREFIX)", "\" -u -- \"",
			target, "\"\n");
}

static int _uninstall_include(Configure * configure, FILE * fp,
		String const * include)
{
	String const * install;

	if((install = _makefile_get_config(configure, include, "install"))
			== NULL)
		install = "$(INCLUDEDIR)";
	_makefile_print(fp, "%s%s/%s\n", "\t$(RM) -- $(DESTDIR)", install,
			include);
	return 0;
}

static int _uninstall_dist(Configure * configure, FILE * fp,
		String const * dist)
{
	String const * install;

	if((install = _makefile_get_config(configure, dist, "install")) == NULL)
		return 0;
	_makefile_print(fp, "%s%s/%s\n", "\t$(RM) -- $(DESTDIR)", install,
			dist);
	return 0;
}


/* accessors */
/* makefile_get_config */
static String const * _makefile_get_config(Configure * configure,
		String const * section, String const * variable)
{
	return configure_get_config(configure, section, variable);
}


/* makefile_is_flag_set */
static unsigned int _makefile_is_flag_set(Configure * configure,
		unsigned int flag)
{
	return configure_is_flag_set(configure, flag);
}


/* makefile_is_phony */
static int _makefile_is_phony(Configure * configure, char const * target)
{
	String const * p;

	if((p = _makefile_get_config(configure, target, "phony")) != NULL
			&& strtol(p, NULL, 10) == 1)
		return 1;
	return 0;
}


/* useful */
/* makefile_expand */
static int _makefile_expand(FILE * fp, char const * field)
{
	String * q;
	int res;

	if((q = string_new(field)) == NULL
			|| string_replace(&q, ",", " ") != 0)
	{
		string_delete(q);
		return -1;
	}
	res = _makefile_print(fp, " %s", q);
	string_delete(q);
	return (res >= 0) ? 0 : -1;
}


/* makefile_link */
static int _makefile_link(FILE * fp, int symlink, char const * link,
		char const * path)
{
	_makefile_print(fp, "\t$(LN)%s -- %s %s\n", symlink ? " -s" : "", link,
			path);
	return 0;
}


/* makefile_output_program */
static int _makefile_output_program(Configure * configure, FILE * fp,
		String const * name)
{
	int ret;
	String const * value;
	String * upper;

	if((upper = string_new(name)) == NULL)
		return -1;
	string_toupper(upper);
	value = configure_get_program(configure, name);
	ret = _makefile_output_variable(fp, upper, value);
	string_delete(upper);
	return ret;
}


/* makefile_output_variable */
static int _makefile_output_variable(FILE * fp, String const * name,
		String const * value)
{
	int res;
	char const * align;
	char const * equals;

	if(fp == NULL)
		return 0;
	if(name == NULL)
		return -1;
	if(value == NULL)
		value = "";
	align = (strlen(name) >= 8) ? "" : "\t";
	equals = (strlen(value) > 0) ? "= " : "=";
	res = _makefile_print(fp, "%s%s%s%s\n", name, align, equals, value);
	return (res >= 0) ? 0 : -1;
}


/* makefile_mkdir */
static int _makefile_mkdir(FILE * fp, char const * directory)
{
	/* FIXME keep track of the directories created */
	return _makefile_print(fp, "%s%s\n", "\t$(MKDIR) $(DESTDIR)",
			directory);
}


/* makefile_print */
static int _makefile_print(FILE * fp, char const * format, ...)
{
	int ret;
	va_list ap;

	va_start(ap, format);
	if(fp == NULL)
		ret = vsnprintf(NULL, 0, format, ap);
	else
		ret = vfprintf(fp, format, ap);
	va_end(ap);
	return ret;
}


/* makefile_remove */
static int _makefile_remove(FILE * fp, int recursive, ...)
{
	va_list ap;
	char const * sep = " -- ";
	char const * p;

	_makefile_print(fp, "\t$(RM)%s", recursive ? " -r" : "");
	va_start(ap, recursive);
	while((p = va_arg(ap, char const * )) != NULL)
	{
		_makefile_print(fp, "%s%s", sep, p);
		sep = " ";
	}
	va_end(ap);
	_makefile_print(fp, "%c", '\n');
	return 0;
}


/* makefile_subdirs */
static int _makefile_subdirs(FILE * fp, char const * target)
{
	if(target != NULL)
		_makefile_print(fp,
				"\t@for i in $(SUBDIRS); do (cd \"$$i\" && \\\n"
				"\t\tif [ -n \"$(OBJDIR)\" ]; then \\\n"
				"\t\t$(MAKE) OBJDIR=\"$(OBJDIR)$$i/\" %s; \\\n"
				"\t\telse $(MAKE) %s; fi) || exit; done\n",
				target, target);
	else
		_makefile_print(fp, "%s",
				"\t@for i in $(SUBDIRS); do (cd \"$$i\" && \\\n"
				"\t\tif [ -n \"$(OBJDIR)\" ]; then \\\n"
				"\t\t([ -d \"$(OBJDIR)$$i\" ]"
				" || $(MKDIR) -- \"$(OBJDIR)$$i\") && \\\n"
				"\t\t$(MAKE) OBJDIR=\"$(OBJDIR)$$i/\"; \\\n"
				"\t\telse $(MAKE); fi) || exit; done\n");
	return 0;
}


/* makefile_target */
static int _makefile_target(FILE * fp, char const * target, ...)
{
	va_list ap;
	char const * sep = " ";
	char const * p;

	if(target == NULL)
		return -1;
	_makefile_print(fp, "\n%s:", target);
	va_start(ap, target);
	while((p = va_arg(ap, char const *)) != NULL)
		_makefile_print(fp, "%s%s", sep, p);
	va_end(ap);
	_makefile_print(fp, "%c", '\n');
	return 0;
}


#ifdef WITH_UNUSED
/* makefile_targetv */
static int _makefile_targetv(FILE * fp, char const * target,
		char const ** depends)
{
	char const ** p;

	if(target == NULL)
		return -1;
	_makefile_print(fp, "\n%s:", target);
	if(depends != NULL)
		for(p = depends; *p != NULL; p++)
			_makefile_print(fp, " %s", *p);
	_makefile_print(fp, "%c", '\n');
	return 0;
}
#endif
