/* $Id$ */
/* Copyright (c) 2006-2019 Pierre Pronchery <khorben@defora.org> */
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
#include "common.h"
#include "settings.h"
#include "configure.h"
#include "../config.h"

#ifndef PROGNAME
# define PROGNAME PACKAGE
#endif


/* private */
/* types */
typedef struct _Makefile
{
	Configure * configure;
	FILE * fp;
} Makefile;


/* prototypes */
/* accessors */
static String const * _makefile_get_config(Makefile * makefile,
		String const * section, String const * variable);
static TargetType _makefile_get_type(Makefile * makefile,
		String const * target);

static int _makefile_is_enabled(Makefile * makefile, char const * target);
static unsigned int _makefile_is_flag_set(Makefile * makefile,
		unsigned int flag);
static int _makefile_is_phony(Makefile * makefile, char const * target);

/* useful */
static int _makefile_expand(Makefile * makefile, char const * field);
static int _makefile_link(Makefile * makefile, int symlink, char const * link,
		char const * path);
static int _makefile_output_extension(Makefile * makefile,
		String const * extension);
static int _makefile_output_path(Makefile * makefile, String const * path);
static int _makefile_output_program(Makefile * makefile, String const * name,
		unsigned int override);
static int _makefile_output_variable(Makefile * makefile, String const * name,
		String const * value);
static int _makefile_mkdir(Makefile * makefile, char const * directory);
static int _makefile_print(Makefile * makefile, char const * format, ...);
static int _makefile_print_escape(Makefile * makefile, char const * str);
static int _makefile_print_escape_variable(Makefile * makefile,
		char const * str);
static int _makefile_remove(Makefile * makefile, int recursive, ...);
static int _makefile_subdirs(Makefile * makefile, char const * target);
static int _makefile_target(Makefile * makefile, char const * target, ...);
#ifdef WITH_UNUSED
static int _makefile_targetv(Makefile * makefile, char const * target,
		char const ** depends);
#endif


/* functions */
/* makefile */
static int _makefile_write(Makefile * makefile, configArray * ca,
	       	int from, int to);

int makefile(Configure * configure, String const * directory, configArray * ca,
	       	int from, int to)
		
{
	Makefile makefile;
	String * filename;
	int ret = 0;

	if(directory == NULL || string_length(directory) == 0)
		filename = string_new(MAKEFILE);
	else
		filename = string_new_append(directory, "/", MAKEFILE, NULL);
	if(filename == NULL)
		return -1;
	makefile.configure = configure;
	makefile.fp = NULL;
	if(!_makefile_is_flag_set(&makefile, PREFS_n)
			&& (makefile.fp = fopen(filename, "w")) == NULL)
		ret = configure_error(1, "%s: %s", filename, strerror(errno));
	else
	{
		if(_makefile_is_flag_set(&makefile, PREFS_v))
			printf("%s%s\n", "Creating ", filename);
		ret |= _makefile_write(&makefile, ca, from, to);
		if(makefile.fp != NULL)
			fclose(makefile.fp);
	}
	string_delete(filename);
	return ret;
}

static int _write_variables(Makefile * makefile);
static int _write_targets(Makefile * makefile);
static int _write_objects(Makefile * makefile);
static int _write_clean(Makefile * makefile);
static int _write_distclean(Makefile * makefile);
static int _write_dist(Makefile * makefile, configArray * ca, int from, int to);
static int _write_distcheck(Makefile * makefile);
static int _write_install(Makefile * makefile);
static int _write_phony(Makefile * makefile,
		char const ** targets);
static int _write_uninstall(Makefile * makefile);
static int _makefile_write(Makefile * makefile, configArray * ca,
	       	int from, int to)
{
	char const * depends[9] = { "all" };
	size_t i = 1;

	if(_write_variables(makefile) != 0
			|| _write_targets(makefile) != 0
			|| _write_objects(makefile) != 0
			|| _write_clean(makefile) != 0
			|| _write_distclean(makefile) != 0
			|| _write_dist(makefile, ca, from, to) != 0
			|| _write_distcheck(makefile) != 0
			|| _write_install(makefile) != 0
			|| _write_uninstall(makefile) != 0)
		return 1;
	if(_makefile_get_config(makefile, NULL, "subdirs") != NULL)
		depends[i++] = "subdirs";
	depends[i++] = "clean";
	depends[i++] = "distclean";
	if(_makefile_get_config(makefile, NULL, "package") != NULL
			&& _makefile_get_config(makefile, NULL,
				"version") != NULL)
	{
		depends[i++] = "dist";
		depends[i++] = "distcheck";
	}
	depends[i++] = "install";
	depends[i++] = "uninstall";
	depends[i++] = NULL;
	return _write_phony(makefile, depends);
}

static int _variables_package(Makefile * makefile,
		String const * directory);
static int _variables_print(Makefile * makefile,
	       	char const * input, char const * output);
static int _variables_dist(Makefile * makefile);
static int _variables_targets(Makefile * makefile);
static int _variables_targets_library(Makefile * makefile,
		char const * target);
static int _variables_executables(Makefile * makefile);
static int _variables_includes(Makefile * makefile);
static int _variables_subdirs(Makefile * makefile);
static int _write_variables(Makefile * makefile)
{
	int ret = 0;
	String const * directory;

	directory = _makefile_get_config(makefile, NULL, "directory");
	ret |= _variables_package(makefile, directory);
	ret |= _variables_print(makefile, "subdirs", "SUBDIRS");
	ret |= _variables_dist(makefile);
	ret |= _variables_targets(makefile);
	ret |= _variables_executables(makefile);
	ret |= _variables_includes(makefile);
	ret |= _variables_subdirs(makefile);
	_makefile_print(makefile, "\n");
	return ret;
}

static int _variables_package(Makefile * makefile,
		String const * directory)
{
	String const * package;
	String const * version;
	String const * p;

	if((package = _makefile_get_config(makefile, NULL, "package")) == NULL)
		return 0;
	if(_makefile_is_flag_set(makefile, PREFS_v))
		printf("%s%s", "Package: ", package);
	if((version = _makefile_get_config(makefile, NULL, "version")) == NULL)
	{
		if(_makefile_is_flag_set(makefile, PREFS_v))
			fputc('\n', stdout);
		fprintf(stderr, "%s%s%s", PROGNAME ": ", directory,
				": \"package\" needs \"version\"\n");
		return 1;
	}
	if(_makefile_is_flag_set(makefile, PREFS_v))
		printf(" %s\n", version);
	_makefile_output_variable(makefile, "PACKAGE", package);
	_makefile_output_variable(makefile, "VERSION", version);
	if((p = _makefile_get_config(makefile, NULL, "config")) != NULL)
		return settings(makefile->configure, directory, package,
				version);
	return 0;
}

static int _variables_print(Makefile * makefile,
		char const * input, char const * output)
{
	String const * p;
	String * prints;
	String * q;
	unsigned long i;
	char c;

	if((p = _makefile_get_config(makefile, NULL, input)) == NULL)
		return 0;
	if((prints = string_new(p)) == NULL)
		return 1;
	q = prints;
	_makefile_print(makefile, "%s%s", output, "\t=");
	for(i = 0;; i++)
	{
		if(prints[i] != ',' && prints[i] != '\0')
			continue;
		c = prints[i];
		prints[i] = '\0';
		if(strchr(prints, ' ') != NULL)
			_makefile_print(makefile, " \"%s\"", prints);
		else
			_makefile_print(makefile, " %s", prints);
		if(c == '\0')
			break;
		prints += i + 1;
		i = 0;
	}
	_makefile_print(makefile, "\n");
	string_delete(q);
	return 0;
}

static int _variables_dist(Makefile * makefile)
{
	String const * p;
	String * dist;
	String * q;
	size_t i;
	char c;

	if((p = _makefile_get_config(makefile, NULL, "dist")) == NULL)
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
		if(_makefile_get_config(makefile, dist, "install") != NULL)
		{
			/* FIXME may still need to be output */
			if(_makefile_get_config(makefile, NULL, "targets")
					== NULL)
			{
				_makefile_output_path(makefile, "objdir");
				_makefile_output_path(makefile, "prefix");
				_makefile_output_path(makefile, "destdir");
			}
			_makefile_output_program(makefile, "mkdir", 0);
			_makefile_output_program(makefile, "install", 0);
			_makefile_output_program(makefile, "rm", 0);
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

static int _variables_targets(Makefile * makefile)
{
	int ret = 0;
	String const * p;
	String * prints;
	String * q;
	size_t i;
	char c;
	String const * type;
	int phony;

	if((p = _makefile_get_config(makefile, NULL, "targets")) == NULL)
		return 0;
	if((prints = string_new(p)) == NULL)
		return 1;
	q = prints;
	_makefile_print(makefile, "%s", "TARGETS\t=");
	for(i = 0;; i++)
	{
		if(prints[i] != ',' && prints[i] != '\0')
			continue;
		c = prints[i];
		prints[i] = '\0';
		if((type = _makefile_get_config(makefile, prints, "type"))
			       	== NULL)
			_makefile_print(makefile, " %s", prints);
		else if(_makefile_is_enabled(makefile, prints) != 0)
			switch(enum_string(TT_LAST, sTargetType, type))
			{
				case TT_BINARY:
					_makefile_print(makefile, "%s",
							" $(OBJDIR)");
					_makefile_print_escape(makefile,
							prints);
					_makefile_print(makefile, "%s",
							"$(EXEEXT)");
					break;
				case TT_COMMAND:
					phony = _makefile_is_phony(makefile,
							prints);
					_makefile_print(makefile, " %s", phony
							? "" : "$(OBJDIR)");
					_makefile_print_escape(makefile,
							prints);
					break;
				case TT_LIBRARY:
					ret |= _variables_targets_library(
							makefile, prints);
					break;
				case TT_LIBTOOL:
					_makefile_print(makefile, "%s",
							" $(OBJDIR)");
					_makefile_print_escape(makefile,
							prints);
					_makefile_print(makefile, "%s", ".la");
					break;
				case TT_OBJECT:
				case TT_UNKNOWN:
					_makefile_print(makefile, " $(OBJDIR)");
					_makefile_print_escape(makefile,
							prints);
					break;
				case TT_SCRIPT:
					phony = _makefile_is_phony(makefile,
							prints);
					_makefile_print(makefile, " %s", phony
							? "" : "$(OBJDIR)");
					_makefile_print_escape(makefile,
							prints);
					break;
				case TT_PLUGIN:
					_makefile_print(makefile, "%s",
							" $(OBJDIR)");
					_makefile_print_escape(makefile,
							prints);
					_makefile_print(makefile, "%s",
							"$(SOEXT)");
					break;
			}
		if(c == '\0')
			break;
		prints += i + 1;
		i = 0;
	}
	_makefile_print(makefile, "\n");
	string_delete(q);
	return ret;
}

static int _variables_targets_library(Makefile * makefile,
		char const * target)
{
	String * soname;
	String const * p;
	HostOS os;

	if((p = _makefile_get_config(makefile, target, "soname")) != NULL)
		soname = string_new(p);
	else if(configure_get_os(makefile->configure) == HO_MACOSX)
		/* versioning is different on MacOS X */
		soname = string_new_append(target, ".0.0$(SOEXT)", NULL);
	else if(configure_get_os(makefile->configure) == HO_WIN32)
		/* and on Windows */
		soname = string_new_append(target, "$(SOEXT)", NULL);
	else
		soname = string_new_append(target, "$(SOEXT)", ".0", NULL);
	if(soname == NULL)
		return 1;
	if(configure_can_library_static(makefile->configure))
		/* generate a static library */
		_makefile_print(makefile, "%s%s%s", " $(OBJDIR)", target, ".a");
	if((os = configure_get_os(makefile->configure)) == HO_MACOSX)
		_makefile_print(makefile, "%s%s%s%s%s%s%s", " $(OBJDIR)",
				soname, " $(OBJDIR)", target,
				".0$(SOEXT) $(OBJDIR)", target, "$(SOEXT)");
	else if(os == HO_WIN32)
		_makefile_print(makefile, "%s%s", " $(OBJDIR)", soname);
	else
		_makefile_print(makefile, "%s%s%s%s%s%s%s", " $(OBJDIR)",
				soname, ".0 $(OBJDIR)", soname, " $(OBJDIR)",
				target, "$(SOEXT)");
	string_delete(soname);
	return 0;
}

static void _executables_variables(Makefile * makefile,
	       	String const * target, char * done);
static int _variables_executables(Makefile * makefile)
{
	char done[TT_LAST]; /* FIXME even better if'd be variable by variable */
	String const * targets;
	String const * includes;
	String const * package;
	String * p;
	String * q;
	size_t i;
	char c;

	memset(&done, 0, sizeof(done));
	targets = _makefile_get_config(makefile, NULL, "targets");
	includes = _makefile_get_config(makefile, NULL, "includes");
	package = _makefile_get_config(makefile, NULL, "package");
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
			_executables_variables(makefile, p, done);
			if(c == '\0')
				break;
			p += i + 1;
			i = 0;
		}
		string_delete(q);
	}
	else if(includes != NULL)
	{
		_makefile_output_path(makefile, "objdir");
		_makefile_output_path(makefile, "prefix");
		_makefile_output_path(makefile, "destdir");
	}
	if(targets != NULL || includes != NULL || package != NULL)
	{
		_makefile_output_program(makefile, "rm", 0);
		_makefile_output_program(makefile, "ln", 0);
	}
	if(package != NULL)
	{
		_makefile_output_program(makefile, "tar", 0);
		_makefile_output_extension(makefile, "tgz");
		_makefile_output_program(makefile, "mkdir", 0);
	}
	if(targets != NULL || includes != NULL)
	{
		if(package == NULL)
			_makefile_output_program(makefile, "mkdir", 0);
		_makefile_output_program(makefile, "install", 0);
	}
	return 0;
}

static void _variables_binary(Makefile * makefile, char * done);
static void _variables_library(Makefile * makefile, char * done);
static void _variables_library_static(Makefile * makefile);
static void _variables_libtool(Makefile * makefile, char * done);
static void _variables_script(Makefile * makefile, char * done);
static void _executables_variables(Makefile * makefile,
	       	String const * target, char * done)
{
	String const * type;
	TargetType tt;

	if((type = _makefile_get_config(makefile, target, "type")) == NULL)
		return;
	if(done[(tt = enum_string(TT_LAST, sTargetType, type))])
		return;
	switch(tt)
	{
		case TT_BINARY:
			_variables_binary(makefile, done);
			done[TT_OBJECT] = 1;
			break;
		case TT_COMMAND:
			break;
		case TT_OBJECT:
			_variables_binary(makefile, done);
			done[TT_BINARY] = 1;
			break;
		case TT_LIBRARY:
			_variables_library(makefile, done);
			done[TT_PLUGIN] = 1;
			break;
		case TT_PLUGIN:
			_variables_library(makefile, done);
			done[TT_LIBRARY] = 1;
			break;
		case TT_LIBTOOL:
			_variables_libtool(makefile, done);
			break;
		case TT_SCRIPT:
			_variables_script(makefile, done);
			break;
		case TT_UNKNOWN:
			break;
	}
	done[tt] = 1;
	return;
}

static void _targets_asflags(Makefile * makefile);
static void _targets_cflags(Makefile * makefile);
static void _targets_cxxflags(Makefile * makefile);
static void _targets_exeext(Makefile * makefile);
static void _targets_ldflags(Makefile * makefile);
static void _targets_jflags(Makefile * makefile);
static void _targets_vflags(Makefile * makefile);
static void _binary_ldflags(Makefile * makefile, String const * ldflags);
static void _variables_binary(Makefile * makefile, char * done)
{
	if(!done[TT_LIBRARY] && !done[TT_SCRIPT])
	{
		_makefile_output_path(makefile, "objdir");
		_makefile_output_path(makefile, "prefix");
		_makefile_output_path(makefile, "destdir");
	}
	_makefile_output_path(makefile, "bindir");
	_makefile_output_path(makefile, "sbindir");
	if(!done[TT_LIBRARY])
	{
		_targets_asflags(makefile);
		_targets_cflags(makefile);
		_targets_cxxflags(makefile);
		_targets_ldflags(makefile);
		_targets_jflags(makefile);
		_targets_vflags(makefile);
		_targets_exeext(makefile);
	}
}

static void _targets_asflags(Makefile * makefile)
{
	String const * as;
	String const * asf;
	String const * asff;

	as = _makefile_get_config(makefile, NULL, "as");
	asff = _makefile_get_config(makefile, NULL, "asflags_force");
	asf = _makefile_get_config(makefile, NULL, "asflags");
	if(as != NULL || asff != NULL || asf != NULL)
	{
		_makefile_output_variable(makefile, "AS", (as != NULL) ? as
				: configure_get_program(makefile->configure,
					"as"));
		_makefile_output_variable(makefile, "ASFLAGSF", asff);
		_makefile_output_variable(makefile, "ASFLAGS", asf);
	}
}

static void _targets_cflags(Makefile * makefile)
{
	String const * cc;
	String const * cff;
	String const * cf;
	String const * cppf;
	String const * cpp;
	String * p;
	HostOS os;

	cppf = _makefile_get_config(makefile, NULL, "cppflags_force");
	cpp = _makefile_get_config(makefile, NULL, "cppflags");
	cff = _makefile_get_config(makefile, NULL, "cflags_force");
	cf = _makefile_get_config(makefile, NULL, "cflags");
	cc = _makefile_get_config(makefile, NULL, "cc");
	if(cppf == NULL && cpp == NULL && cff == NULL && cf == NULL
			&& cc == NULL)
		return;
	_makefile_output_program(makefile, "cc", 1);
	_makefile_output_variable(makefile, "CPPFLAGSF", cppf);
	_makefile_output_variable(makefile, "CPPFLAGS", cpp);
	p = NULL;
	if((os = configure_get_os(makefile->configure)) == HO_GNU_LINUX
			&& cff != NULL && string_find(cff, "-ansi"))
		p = string_new_append(cff, " -D _GNU_SOURCE");
	_makefile_output_variable(makefile, "CFLAGSF", (p != NULL) ? p : cff);
	string_delete(p);
	p = NULL;
	if(os == HO_GNU_LINUX && cf != NULL && string_find(cf, "-ansi"))
		p = string_new_append(cf, " -D _GNU_SOURCE");
	_makefile_output_variable(makefile, "CFLAGS", (p != NULL) ? p : cf);
	string_delete(p);
}

static void _targets_cxxflags(Makefile * makefile)
{
	String const * cxx;
	String const * cxxff;
	String const * cxxf;

	cxx = _makefile_get_config(makefile, NULL, "cxx");
	cxxff = _makefile_get_config(makefile, NULL, "cxxflags_force");
	cxxf = _makefile_get_config(makefile, NULL, "cxxflags");
	if(cxx != NULL || cxxff != NULL || cxxf != NULL)
		_makefile_output_program(makefile, "cxx", 1);
	if(cxxff != NULL)
	{
		_makefile_print(makefile, "%s%s", "CXXFLAGSF= ", cxxff);
		if(configure_get_os(makefile->configure) == HO_GNU_LINUX
				&& string_find(cxxff, "-ansi"))
			_makefile_print(makefile, "%s", " -D _GNU_SOURCE");
		_makefile_print(makefile, "\n");
	}
	if(cxxf != NULL)
	{
		_makefile_print(makefile, "%s%s", "CXXFLAGS= ", cxxf);
		if(configure_get_os(makefile->configure) == HO_GNU_LINUX
				&& string_find(cxxf, "-ansi"))
			_makefile_print(makefile, "%s", " -D _GNU_SOURCE");
		_makefile_print(makefile, "\n");
	}
}

static void _targets_exeext(Makefile * makefile)
{
	_makefile_output_extension(makefile, "exe");
}

static void _targets_ldflags(Makefile * makefile)
{
	String const * p;

	if((p = _makefile_get_config(makefile, NULL, "ldflags_force")) != NULL)
	{
		_makefile_print(makefile, "%s", "LDFLAGSF=");
		_binary_ldflags(makefile, p);
		_makefile_print(makefile, "\n");
	}
	if((p = _makefile_get_config(makefile, NULL, "ldflags")) != NULL)
	{
		_makefile_print(makefile, "%s", "LDFLAGS\t=");
		_binary_ldflags(makefile, p);
		_makefile_print(makefile, "\n");
	}
}

static void _targets_jflags(Makefile * makefile)
{
	String const * j;
	String const * jff;
	String const * jf;

	j = _makefile_get_config(makefile, NULL, "javac");
	jff = _makefile_get_config(makefile, NULL, "jflags_force");
	jf = _makefile_get_config(makefile, NULL, "jflags");
	if(j != NULL || jff != NULL || jf != NULL)
		_makefile_output_program(makefile, "javac", 1);
	if(jff != NULL)
		_makefile_output_variable(makefile, "JFLAGSF", jff);
	if(jf != NULL)
		_makefile_output_variable(makefile, "JFLAGS", jf);
}

static void _targets_vflags(Makefile * makefile)
{
	String const * v;
	String const * vff;
	String const * vf;

	v = _makefile_get_config(makefile, NULL, "verilog");
	vff = _makefile_get_config(makefile, NULL, "vflags_force");
	vf = _makefile_get_config(makefile, NULL, "vflags");
	if(v != NULL || vff != NULL || vf != NULL)
		_makefile_output_program(makefile, "verilog", 1);
	if(vff != NULL)
		_makefile_output_variable(makefile, "VFLAGSF", vff);
	if(vf != NULL)
		_makefile_output_variable(makefile, "VFLAGS", vf);
}

static void _binary_ldflags(Makefile * makefile, String const * ldflags)
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
		_makefile_print(makefile, " %s%s", ldflags, "\n");
		return;
	}
	switch(configure_get_os(makefile->configure))
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
		snprintf(buf, sizeof(buf), "-l%s", libs[i]);
		if((q = string_find(p, buf)) == NULL)
			continue;
		memmove(q, q + strlen(buf), strlen(q) - strlen(buf) + 1);
	}
	_makefile_print(makefile, " %s", p);
	string_delete(p);
}

static void _variables_library(Makefile * makefile, char * done)
{
	String const * p;

	if(!done[TT_LIBRARY] && !done[TT_SCRIPT])
	{
		_makefile_output_path(makefile, "objdir");
		_makefile_output_path(makefile, "prefix");
		_makefile_output_path(makefile, "destdir");
	}
	_makefile_output_path(makefile, "libdir");
	if(!done[TT_BINARY])
	{
		_targets_asflags(makefile);
		_targets_cflags(makefile);
		_targets_cxxflags(makefile);
		_targets_ldflags(makefile);
		_targets_vflags(makefile);
		_targets_exeext(makefile);
	}
	if(configure_can_library_static(makefile->configure))
		_variables_library_static(makefile);
	if((p = _makefile_get_config(makefile, NULL, "ld")) == NULL)
		_makefile_output_program(makefile, "ccshared", 0);
	else
		_makefile_output_variable(makefile, "CCSHARED", p);
	_makefile_output_extension(makefile, "so");
}

static void _variables_library_static(Makefile * makefile)
{
	String const * p;

	if((p = _makefile_get_config(makefile, NULL, "ar")) == NULL)
		_makefile_output_program(makefile, "ar", 0);
	else
		_makefile_output_variable(makefile, "AR", p);
	_makefile_output_variable(makefile, "ARFLAGS", "-rc");
	if((p = _makefile_get_config(makefile, NULL, "ranlib")) == NULL)
		_makefile_output_program(makefile, "ranlib", 0);
	else
		_makefile_output_variable(makefile, "RANLIB", p);
}

static void _variables_libtool(Makefile * makefile, char * done)
{
	_variables_library(makefile, done);
	if(!done[TT_LIBTOOL])
		_makefile_output_program(makefile, "libtool", 1);
}

static void _variables_script(Makefile * makefile, char * done)
{
	if(!done[TT_BINARY] && !done[TT_LIBRARY] && !done[TT_SCRIPT])
	{
		_makefile_output_path(makefile, "objdir");
		_makefile_output_path(makefile, "prefix");
		_makefile_output_path(makefile, "destdir");
	}
}

static int _variables_includes(Makefile * makefile)
{
	String const * includes;

	if((includes = _makefile_get_config(makefile, NULL, "includes"))
			== NULL)
		return 0;
	if(makefile->fp == NULL)
		return 0;
	_makefile_output_path(makefile, "includedir");
	return 0;
}

static int _variables_subdirs(Makefile * makefile)
{
	String const * sections[] = { "dist" };
	size_t i;
	String * p;
	String const * q;

	if(_makefile_get_config(makefile, NULL, "subdirs") == NULL
			|| _makefile_get_config(makefile, NULL, "package") != NULL
			|| _makefile_get_config(makefile, NULL, "targets") != NULL
			|| _makefile_get_config(makefile, NULL, "includes") != NULL)
		return 0;
	for(i = 0; i < sizeof(sections) / sizeof(*sections); i++)
	{
		if((q = _makefile_get_config(makefile, NULL, sections[i]))
				== NULL)
			continue;
		if((p = strdup(q)) == NULL)
			return -1;
		for(q = strtok(p, ","); q != NULL; q = strtok(NULL, ","))
			if(_makefile_get_config(makefile, q, "install")
					!= NULL)
				break;
		free(p);
		if(q != NULL)
			return 0;
	}
	return _makefile_output_program(makefile, "mkdir", 0);
}

static int _targets_all(Makefile * makefile);
static int _targets_subdirs(Makefile * makefile);
static int _targets_target(Makefile * makefile, String const * target);
static int _write_targets(Makefile * makefile)
{
	int ret = 0;
	String const * p;
	String * targets;
	String * q;
	size_t i;
	char c;

	if(_targets_all(makefile) != 0
			|| _targets_subdirs(makefile) != 0)
		return 1;
	if((p = _makefile_get_config(makefile, NULL, "targets")) == NULL)
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
		ret |= _targets_target(makefile, targets);
		if(c == '\0')
			break;
		targets += i + 1;
		i = 0;
	}
	string_delete(q);
	return ret;
}

static int _targets_all(Makefile * makefile)
{
	char const * depends[] = { NULL, NULL };
	size_t i = 0;

	if(_makefile_get_config(makefile, NULL, "subdirs") != NULL)
		depends[i++] = "subdirs";
	if(_makefile_get_config(makefile, NULL, "targets") != NULL)
		depends[i++] = "$(TARGETS)";
	_makefile_target(makefile, "all", depends[0], depends[1], NULL);
	return 0;
}

static int _targets_subdirs(Makefile * makefile)
{
	String const * subdirs;

	if((subdirs = _makefile_get_config(makefile, NULL, "subdirs")) != NULL)
	{
		_makefile_target(makefile, "subdirs", NULL);
		_makefile_subdirs(makefile, NULL);
	}
	return 0;
}

static int _target_objs(Makefile * makefile, String const * target);
static int _target_binary(Makefile * makefile, String const * target);
static int _target_command(Makefile * makefile, String const * target);
static int _target_library(Makefile * makefile, String const * target);
static int _target_library_static(Makefile * makefile, String const * target);
static int _target_libtool(Makefile * makefile, String const * target);
static int _target_object(Makefile * makefile, String const * target);
static int _target_plugin(Makefile * makefile, String const * target);
static int _target_script(Makefile * makefile, String const * target);
static int _targets_target(Makefile * makefile, String const * target)
{
	String const * type;
	TargetType tt;

	if((type = _makefile_get_config(makefile, target, "type")) == NULL)
	{
		fprintf(stderr, "%s%s%s", PROGNAME ": ", target,
				": no type defined for target\n");
		return 1;
	}
	tt = enum_string(TT_LAST, sTargetType, type);
	switch(tt)
	{
		case TT_BINARY:
			return _target_binary(makefile, target);
		case TT_COMMAND:
			return _target_command(makefile, target);
		case TT_LIBRARY:
			return _target_library(makefile, target);
		case TT_LIBTOOL:
			return _target_libtool(makefile, target);
		case TT_OBJECT:
			return _target_object(makefile, target);
		case TT_PLUGIN:
			return _target_plugin(makefile, target);
		case TT_SCRIPT:
			return _target_script(makefile, target);
		case TT_UNKNOWN:
			fprintf(stderr, "%s%s%s", PROGNAME ": ", target,
					": unknown type for target\n");
			return 1;
	}
	return 0;
}

static int _objs_source(Makefile * makefile, String * source, TargetType tt);
static int _target_objs(Makefile * makefile, String const * target)
{
	int ret = 0;
	String const * p;
	TargetType tt = TT_UNKNOWN;
	String * sources;
	String * q;
	size_t i;
	char c;

	if((p = _makefile_get_config(makefile, target, "type")) != NULL)
		tt = enum_string(TT_LAST, sTargetType, p);
	if((p = _makefile_get_config(makefile, target, "sources")) == NULL)
		/* a binary target may have no sources */
		return 0;
	if((sources = string_new(p)) == NULL)
		return 1;
	q = sources;
	_makefile_print(makefile, "\n");
	_makefile_print_escape_variable(makefile, target);
	_makefile_print(makefile, "%s", "_OBJS =");
	for(i = 0; ret == 0; i++)
	{
		if(sources[i] != ',' && sources[i] != '\0')
			continue;
		c = sources[i];
		sources[i] = '\0';
		ret = _objs_source(makefile, sources, tt);
		if(c == '\0')
			break;
		sources += i + 1;
		i = 0;
	}
	_makefile_print(makefile, "\n");
	string_delete(q);
	return ret;
}

static int _objs_source(Makefile * makefile, String * source, TargetType tt)
{
	int ret = 0;
	String const * extension;
	size_t len;

	if((extension = source_extension(source)) == NULL)
	{
		fprintf(stderr, "%s%s%s", PROGNAME ": ", source,
				": no extension for source\n");
		return 1;
	}
	len = string_length(source) - string_length(extension) - 1;
	source[len] = '\0';
	switch(source_type(extension))
	{
		case OT_ASM_SOURCE:
		case OT_ASMPP_SOURCE:
		case OT_C_SOURCE:
		case OT_CXX_SOURCE:
		case OT_OBJC_SOURCE:
		case OT_OBJCXX_SOURCE:
			_makefile_print(makefile, "%s", " $(OBJDIR)");
			_makefile_print_escape(makefile, source);
			_makefile_print(makefile, "%s",
					(tt == TT_LIBTOOL) ? ".lo" : ".o");
			break;
		case OT_JAVA_SOURCE:
			_makefile_print(makefile, "%s", " $(OBJDIR)");
			_makefile_print_escape(makefile, source);
			_makefile_print(makefile, "%s", ".class");
			break;
		case OT_VERILOG_SOURCE:
			_makefile_print(makefile, "%s", " $(OBJDIR)");
			_makefile_print_escape(makefile, source);
			_makefile_print(makefile, "%s", ".o");
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

static int _target_flags(Makefile * makefile, String const * target);
static int _target_binary(Makefile * makefile, String const * target)
{
	String const * p;

	if(_target_objs(makefile, target) != 0)
		return 1;
	if(_target_flags(makefile, target) != 0)
		return 1;
	_makefile_print(makefile, "\n");
	/* output the binary target */
	_makefile_print(makefile, "%s", "$(OBJDIR)");
	_makefile_print_escape(makefile, target);
	_makefile_print(makefile, "%s", "$(EXEEXT): $(");
	_makefile_print_escape_variable(makefile, target);
	_makefile_print(makefile, "%s", "_OBJS)");
	if((p = _makefile_get_config(makefile, target, "depends")) != NULL
			&& _makefile_expand(makefile, p) != 0)
		return error_print(PROGNAME);
	_makefile_print(makefile, "\n");
	/* build the binary */
	_makefile_print(makefile, "%s", "\t$(CC) -o $(OBJDIR)");
	_makefile_print_escape(makefile, target);
	_makefile_print(makefile, "%s", "$(EXEEXT) $(");
	_makefile_print_escape_variable(makefile, target);
	_makefile_print(makefile, "%s", "_OBJS) $(");
	_makefile_print_escape_variable(makefile, target);
	_makefile_print(makefile, "%s", "_LDFLAGS)\n");
	return 0;
}

static void _flags_asm(Makefile * makefile, String const * target);
static void _flags_asmpp(Makefile * makefile, String const * target);
static void _flags_c(Makefile * makefile, String const * target);
static void _flags_cxx(Makefile * makefile, String const * target);
static void _flags_java(Makefile * makefile, String const * target);
static void _flags_verilog(Makefile * makefile, String const * target);
static int _target_flags(Makefile * makefile, String const * target)
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
	if((p = _makefile_get_config(makefile, target, "sources")) == NULL
			|| string_length(p) == 0)
	{
		if((p = _makefile_get_config(makefile, target, "type")) != NULL
				&& string_compare(p, "binary") == 0)
			_flags_c(makefile, target);
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
		extension = source_extension(sources);
		if(extension == NULL)
		{
			sources[i] = c;
			continue;
		}
		type = source_type(extension);
		if(!done[type])
			switch(type)
			{
				case OT_ASMPP_SOURCE:
					done[OT_ASMPP_SOURCE] = 1;
					_flags_asmpp(makefile, target);
					if(done[OT_ASM_SOURCE])
						break;
					/* fallback */
				case OT_ASM_SOURCE:
					done[OT_ASM_SOURCE] = 1;
					_flags_asm(makefile, target);
					break;
				case OT_JAVA_SOURCE:
					done[OT_JAVA_SOURCE] = 1;
					_flags_java(makefile, target);
					break;
				case OT_OBJC_SOURCE:
					done[OT_C_SOURCE] = 1;
					/* fallback */
				case OT_C_SOURCE:
					done[OT_OBJC_SOURCE] = 1;
					_flags_c(makefile, target);
					break;
				case OT_OBJCXX_SOURCE:
					done[OT_CXX_SOURCE] = 1;
					/* fallback */
				case OT_CXX_SOURCE:
					done[OT_OBJCXX_SOURCE] = 1;
					_flags_cxx(makefile, target);
					break;
				case OT_VERILOG_SOURCE:
					done[OT_VERILOG_SOURCE] = 1;
					_flags_verilog(makefile, target);
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

static void _flags_asm(Makefile * makefile, String const * target)
{
	String const * p;

	_makefile_print(makefile, "%s%s", target, "_ASFLAGS = $(ASFLAGSF)"
			" $(ASFLAGS)");
	if((p = _makefile_get_config(makefile, target, "asflags")) != NULL)
		_makefile_print(makefile, " %s", p);
	_makefile_print(makefile, "\n");
}

static void _flags_asmpp(Makefile * makefile, String const * target)
{
	String const * p;

	_makefile_print(makefile, "%s%s", target, "_CPPFLAGS = $(CPPFLAGSF)"
			" $(CPPFLAGS)");
	if((p = _makefile_get_config(makefile, target, "cppflags")) != NULL)
		_makefile_print(makefile, " %s", p);
	_makefile_print(makefile, "\n");
}

static void _flags_c(Makefile * makefile, String const * target)
{
	String const * p;

	_makefile_print_escape_variable(makefile, target);
	_makefile_print(makefile, "%s", "_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS)");
	if((p = _makefile_get_config(makefile, target, "cppflags")) != NULL)
		_makefile_print(makefile, " %s", p);
	_makefile_print(makefile, "%s", " $(CFLAGSF) $(CFLAGS)");
	if((p = _makefile_get_config(makefile, target, "cflags")) != NULL)
	{
		_makefile_print(makefile, " %s", p);
		if(configure_get_os(makefile->configure) == HO_GNU_LINUX
				&& string_find(p, "-ansi"))
			_makefile_print(makefile, "%s", " -D _GNU_SOURCE");
	}
	_makefile_print(makefile, "\n");
	_makefile_print_escape_variable(makefile, target);
	_makefile_print(makefile, "%s", "_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)");
	if((p = _makefile_get_config(makefile, target, "ldflags")) != NULL)
		_binary_ldflags(makefile, p);
	_makefile_print(makefile, "\n");
}

static void _flags_cxx(Makefile * makefile, String const * target)
{
	String const * p;

	_makefile_print_escape_variable(makefile, target);
	_makefile_print(makefile, "%s", "_CXXFLAGS = $(CPPFLAGSF) $(CPPFLAGS)");
	if((p = _makefile_get_config(makefile, target, "cppflags")) != NULL)
		_makefile_print(makefile, " %s", p);
	_makefile_print(makefile, "%s", " $(CXXFLAGSF) $(CXXFLAGS)");
	if((p = _makefile_get_config(makefile, target, "cxxflags")) != NULL)
		_makefile_print(makefile, " %s", p);
	_makefile_print(makefile, "\n");
	_makefile_print_escape_variable(makefile, target);
	_makefile_print(makefile, "%s", "_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)");
	if((p = _makefile_get_config(makefile, target, "ldflags")) != NULL)
		_binary_ldflags(makefile, p);
	_makefile_print(makefile, "\n");
}

static void _flags_java(Makefile * makefile, String const * target)
{
	String const * p;

	_makefile_print_escape_variable(makefile, target);
	_makefile_print(makefile, "%s", "_JFLAGS = $(JFLAGSF) $(JFLAGS)");
	if((p = _makefile_get_config(makefile, target, "jflags")) != NULL)
		_makefile_print(makefile, " %s", p);
	_makefile_print(makefile, "\n");
}

static void _flags_verilog(Makefile * makefile, String const * target)
{
	String const * p;

	_makefile_print_escape_variable(makefile, target);
	_makefile_print(makefile, "%s",
			"_VFLAGS = $(VFLAGSF) $(VFLAGS)");
	if((p = _makefile_get_config(makefile, target, "vflags")) != NULL)
		_makefile_print(makefile, " %s", p);
	_makefile_print(makefile, "\n");
}

static void _command_security(Makefile * makefile, String const * target,
		String const * command);
static int _target_command(Makefile * makefile, String const * target)
{
	String const * p;
	int phony;

	phony = _makefile_is_phony(makefile, target);
	_makefile_print(makefile, "\n%s", phony ? "" : "$(OBJDIR)");
	_makefile_print_escape(makefile, target);
	_makefile_print(makefile, ":");
	if((p = _makefile_get_config(makefile, target, "depends")) != NULL
			&& _makefile_expand(makefile, p) != 0)
		return error_print(PROGNAME);
	if((p = _makefile_get_config(makefile, target, "command")) == NULL)
		return error_print(PROGNAME);
	if(_makefile_is_flag_set(makefile, PREFS_S))
		_command_security(makefile, target, p);
	_makefile_print(makefile, "\n\t%s\n", p);
	return 0;
}

static void _command_security(Makefile * makefile, String const * target,
		String const * command)
{
	(void) makefile;

	error_set_print(PROGNAME, 0, "%s: %s%s%s", target, "Command \"",
			command, "\" is executed while compiling");
}

static int _target_library(Makefile * makefile, String const * target)
{
	String const * p;
	String * q;
	String * soname;
	HostOS os;

	if(_target_objs(makefile, target) != 0)
		return 1;
	if(_target_flags(makefile, target) != 0)
		return 1;
	if(configure_can_library_static(makefile->configure)
			/* generate a static library */
			&& _target_library_static(makefile, target) != 0)
		return 1;
	os = configure_get_os(makefile->configure);
	if((p = _makefile_get_config(makefile, target, "soname")) != NULL)
		soname = string_new(p);
	else if(os == HO_MACOSX)
		/* versioning is different on MacOS X */
		soname = string_new_append(target, ".0.0$(SOEXT)", NULL);
	else if(os == HO_WIN32)
		/* and on Windows */
		soname = string_new_append(target, "$(SOEXT)", NULL);
	else
		soname = string_new_append(target, "$(SOEXT)", ".0", NULL);
	if(soname == NULL)
		return 1;
	if(os == HO_MACOSX)
		_makefile_print(makefile, "%s%s", "\n$(OBJDIR)", soname);
	else if(os == HO_WIN32)
		_makefile_print(makefile, "%s%s", "\n$(OBJDIR)", soname);
	else
		_makefile_print(makefile, "%s%s%s", "\n$(OBJDIR)", soname,
				".0");
	_makefile_print(makefile, ": $(");
	_makefile_print_escape_variable(makefile, target);
	_makefile_print(makefile, "%s", "_OBJS)");
	if((p = _makefile_get_config(makefile, target, "depends")) != NULL
			&& _makefile_expand(makefile, p) != 0)
		return error_print(PROGNAME);
	_makefile_print(makefile, "\n");
	/* build the shared library */
	_makefile_print(makefile, "%s%s%s", "\t$(CCSHARED) -o $(OBJDIR)", soname,
			(os != HO_MACOSX && os != HO_WIN32) ? ".0" : "");
	if((p = _makefile_get_config(makefile, target, "install")) != NULL)
	{
		/* soname is not available on MacOS X */
		if(os == HO_MACOSX)
			_makefile_print(makefile, "%s%s%s%s%s",
					" -install_name ", p, "/", target,
					".0$(SOEXT)");
		else if(os != HO_WIN32)
			_makefile_print(makefile, "%s%s", " -Wl,-soname,",
					soname);
	}
	_makefile_print(makefile, "%s", " $(");
	_makefile_print_escape_variable(makefile, target);
	_makefile_print(makefile, "%s", "_OBJS) $(");
	_makefile_print_escape_variable(makefile, target);
	_makefile_print(makefile, "%s", "_LDFLAGS)");
	if((q = string_new_append(target, "$(SOEXT)", NULL)) == NULL)
	{
		string_delete(soname);
		return 1;
	}
	if((p = _makefile_get_config(makefile, q, "ldflags")) != NULL)
		_binary_ldflags(makefile, p);
	string_delete(q);
	_makefile_print(makefile, "\n");
	if(os == HO_MACOSX)
	{
		_makefile_print(makefile, "%s%s%s%s%s", "\n$(OBJDIR)", target,
				".0$(SOEXT): $(OBJDIR)", soname, "\n");
		_makefile_print(makefile, "%s%s%s%s%s\n", "\t$(LN) -s -- ",
				soname, " $(OBJDIR)", target, ".0$(SOEXT)");
		_makefile_print(makefile, "%s%s%s%s\n", "\n$(OBJDIR)", target,
				"$(SOEXT): $(OBJDIR)", soname);
		_makefile_print(makefile, "%s%s%s%s%s", "\t$(LN) -s -- ",
				soname, " $(OBJDIR)", target, "$(SOEXT)\n");
	}
	else if(os != HO_WIN32)
	{
		_makefile_print(makefile, "%s%s%s%s%s", "\n$(OBJDIR)", soname,
				": $(OBJDIR)", soname, ".0\n");
		_makefile_print(makefile, "%s%s%s%s\n", "\t$(LN) -s -- ",
				soname, ".0 $(OBJDIR)", soname);
		_makefile_print(makefile, "%s%s%s%s%s", "\n$(OBJDIR)", target,
				"$(SOEXT): $(OBJDIR)", soname, ".0\n");
		_makefile_print(makefile, "%s%s%s%s%s", "\t$(LN) -s -- ", soname,
				".0 $(OBJDIR)", target, "$(SOEXT)\n");
	}
	string_delete(soname);
	return 0;
}

static int _target_library_static(Makefile * makefile, String const * target)
{
	String const * p;
	String * q;
	size_t len;

	_makefile_print(makefile, "%s%s%s%s%s", "\n$(OBJDIR)", target,
			".a: $(", target, "_OBJS)");
	if((p = _makefile_get_config(makefile, target, "depends")) != NULL
			&& _makefile_expand(makefile, p) != 0)
		return error_print(PROGNAME);
	_makefile_print(makefile, "\n");
	/* build the static library */
	_makefile_print(makefile, "%s", "\t$(AR) $(ARFLAGS) $(OBJDIR)");
	_makefile_print_escape(makefile, target);
	_makefile_print(makefile, "%s", ".a $(");
	_makefile_print_escape_variable(makefile, target);
	_makefile_print(makefile, "%s", "_OBJS)");
	len = strlen(target) + 3;
	if((q = malloc(len)) == NULL)
		return 1;
	snprintf(q, len, "%s.a", target);
	if((p = _makefile_get_config(makefile, q, "ldflags")) != NULL)
		_binary_ldflags(makefile, p);
	free(q);
	_makefile_print(makefile, "%s%s%s",
			"\n\t$(RANLIB) $(OBJDIR)", target, ".a\n");
	return 0;
}

static int _target_libtool(Makefile * makefile, String const * target)
{
	String const * p;

	if(_target_objs(makefile, target) != 0)
		return 1;
	if(_target_flags(makefile, target) != 0)
		return 1;
	_makefile_print(makefile, "%s", "\n$(OBJDIR)");
	_makefile_print_escape(makefile, target);
	_makefile_print(makefile, ".la: $(");
	_makefile_print_escape_variable(makefile, target);
	_makefile_print(makefile, "%s", "_OBJS)\n");
	_makefile_print(makefile, "%s",
			"\t$(LIBTOOL) --mode=link $(CC) -o $(OBJDIR)");
	_makefile_print_escape(makefile, target);
	_makefile_print(makefile, "%s", ".la $(");
	_makefile_print_escape_variable(makefile, target);
	_makefile_print(makefile, "%s", "_OBJS)");
	if((p = _makefile_get_config(makefile, target, "ldflags")) != NULL)
		_binary_ldflags(makefile, p);
	_makefile_print(makefile, "%s", " -rpath $(LIBDIR) $(");
	_makefile_print_escape_variable(makefile, target);
	_makefile_print(makefile, "%s", "_LDFLAGS)\n");
	return 0;
}

static int _target_object(Makefile * makefile, String const * target)
{
	String const * p;
	String const * extension;

	if((p = _makefile_get_config(makefile, target, "sources")) == NULL)
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
	if((extension = source_extension(p)) == NULL)
		return 1;
	switch(source_type(extension))
	{
		case OT_ASM_SOURCE:
			_makefile_print(makefile, "\n%s%s",
					target, "_OBJS = $(OBJDIR)");
			_makefile_print_escape(makefile, target);
			_makefile_print(makefile, "\n%s%s", target, "_ASFLAGS ="
					" $(ASFLAGSF) $(ASFLAGS)");
			if((p = _makefile_get_config(makefile, target,
							"asflags")) != NULL)
				_makefile_print(makefile, " %s", p);
			_makefile_print(makefile, "\n");
			break;
		case OT_ASMPP_SOURCE:
			_makefile_print(makefile, "\n%s%s", target,
					"_OBJS = $(OBJDIR)");
			_makefile_print_escape(makefile, target);
			_makefile_print(makefile, "\n%s%s", target,
					"_CPPFLAGS = $(CPPFLAGSF) $(CPPFLAGS)");
			if((p = _makefile_get_config(makefile, target,
							"cppflags")) != NULL)
				_makefile_print(makefile, " %s", p);
			_makefile_print(makefile, "\n%s%s", target,
					"_ASFLAGS = $(ASFLAGSF) $(ASFLAGS)");
			if((p = _makefile_get_config(makefile, target,
							"asflags")) != NULL)
				_makefile_print(makefile, " %s", p);
			_makefile_print(makefile, "\n");
			break;
		case OT_C_SOURCE:
		case OT_OBJC_SOURCE:
			_makefile_print(makefile, "\n%s%s%s", target,
					"_OBJS = ", "$(OBJDIR)");
			_makefile_print_escape(makefile, target);
			_makefile_print(makefile, "\n%s%s", target, "_CFLAGS ="
					" $(CPPFLAGSF) $(CPPFLAGS)");
			if((p = _makefile_get_config(makefile, target,
							"cppflags")) != NULL)
				_makefile_print(makefile, " %s", p);
			_makefile_print(makefile, "%s", " $(CFLAGSF)"
					" $(CFLAGS)");
			if((p = _makefile_get_config(makefile, target,
							"cflags")) != NULL)
				_makefile_print(makefile, " %s", p);
			_makefile_print(makefile, "\n");
			break;
		case OT_CXX_SOURCE:
		case OT_OBJCXX_SOURCE:
			_makefile_print(makefile, "\n%s%s%s", target,
					"_OBJS = ", "$(OBJDIR)");
			_makefile_print_escape(makefile, target);
			_makefile_print(makefile, "\n%s%s", target,
					"_CXXFLAGS = $(CPPFLAGSF) $(CPPFLAGS)");
			if((p = _makefile_get_config(makefile, target,
							"cppflags")) != NULL)
				_makefile_print(makefile, " %s", p);
			_makefile_print(makefile, "%s", " $(CXXFLAGSF)"
					" $(CXXFLAGS)");
			if((p = _makefile_get_config(makefile, target,
							"cxxflags")) != NULL)
				_makefile_print(makefile, " %s", p);
			_makefile_print(makefile, "\n");
			break;
		case OT_JAVA_SOURCE:
			_makefile_print(makefile, "\n");
			_makefile_print_escape_variable(makefile, target);
			_makefile_print(makefile, "%s", "_OBJS = $(OBJDIR)");
			_makefile_print_escape(makefile, target);
			_makefile_print(makefile, "\n");
			_makefile_print_escape_variable(makefile, target);
			_makefile_print(makefile, "%s",
					"_JFLAGS = $(JFLAGSF) $(JFLAGS)");
			if((p = _makefile_get_config(makefile, target,
							"jflags")) != NULL)
				_makefile_print(makefile, " %s", p);
			_makefile_print(makefile, "\n");
			break;
		case OT_VERILOG_SOURCE:
			_makefile_print(makefile, "\n");
			_makefile_print_escape_variable(makefile, target);
			_makefile_print(makefile, "%s", "_OBJS = $(OBJDIR)");
			_makefile_print_escape(makefile, target);
			_makefile_print(makefile, "\n");
			_makefile_print_escape_variable(makefile, target);
			_makefile_print(makefile, "%s",
					"_VFLAGS = $(VFLAGSF) $(VFLAGS)");
			if((p = _makefile_get_config(makefile, target,
							"vflags")) != NULL)
				_makefile_print(makefile, " %s", p);
			_makefile_print(makefile, "\n");
			break;
		case OT_UNKNOWN:
			fprintf(stderr, "%s%s%s", PROGNAME ": ", target,
					": Unknown source type for object\n");
			return 1;
	}
	return 0;
}

static int _target_plugin(Makefile * makefile, String const * target)
{
	String const * p;
	String * q;

	if(_target_objs(makefile, target) != 0)
		return 1;
	if(_target_flags(makefile, target) != 0)
		return 1;
	_makefile_print(makefile, "%s", "\n$(OBJDIR)");
	_makefile_print_escape(makefile, target);
	_makefile_print(makefile, "%s", "$(SOEXT): $(");
	_makefile_print_escape_variable(makefile, target);
	_makefile_print(makefile, "%s", "_OBJS)");
	if((p = _makefile_get_config(makefile, target, "depends")) != NULL
			&& _makefile_expand(makefile, p) != 0)
		return error_print(PROGNAME);
	_makefile_print(makefile, "\n");
	/* build the plug-in */
	_makefile_print(makefile, "%s", "\t$(CCSHARED) -o $(OBJDIR)");
	_makefile_print_escape(makefile, target);
	_makefile_print(makefile, "%s", "$(SOEXT) $(");
	_makefile_print_escape_variable(makefile, target);
	_makefile_print(makefile, "%s", "_OBJS) $(");
	_makefile_print_escape_variable(makefile, target);
	_makefile_print(makefile, "%s", "_LDFLAGS)");
	if((q = string_new_append(target, "$(SOEXT)", NULL)) == NULL)
		return error_print(PROGNAME);
	if((p = _makefile_get_config(makefile, q, "ldflags")) != NULL)
		_binary_ldflags(makefile, p);
	string_delete(q);
	_makefile_print(makefile, "\n");
	return 0;
}

static int _objects_target(Makefile * makefile,
		String const * target);
static int _write_objects(Makefile * makefile)
{
	String const * p;
	String * targets;
	String * q;
	char c;
	size_t i;
	int ret = 0;

	if((p = _makefile_get_config(makefile, NULL, "targets")) == NULL)
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
		ret += _objects_target(makefile, targets);
		if(c == '\0')
			break;
		targets += i + 1;
		i = 0;
	}
	string_delete(q);
	return ret;
}

static void _script_check(Makefile * makefile, String const * target,
		String const * script);
static int _script_depends(Makefile * makefile, String const * target);
static String * _script_path(Makefile * makefile, String const * script);
static void _script_security(Makefile * makefile, String const * target,
		String const * script);
static int _target_script(Makefile * makefile,
		String const * target)
{
	String const * prefix;
	String const * script;
	String const * flags;
	int phony;

	if((script = _makefile_get_config(makefile, target, "script")) == NULL)
	{
		fprintf(stderr, "%s%s%s", PROGNAME ": ", target,
				": No script for target\n");
		return 1;
	}
	flags = _makefile_get_config(makefile, target, "flags");
	if(makefile->fp == NULL)
		_script_check(makefile, target, script);
	if(_makefile_is_flag_set(makefile, PREFS_S))
		_script_security(makefile, target, script);
	phony = _makefile_is_phony(makefile, target);
	_makefile_print(makefile, "\n%s", phony ? "" : "$(OBJDIR)");
	_makefile_print_escape(makefile, target);
	_makefile_print(makefile, ":");
	_script_depends(makefile, target);
	if((prefix = _makefile_get_config(makefile, target, "prefix")) == NULL)
		prefix = "$(PREFIX)";
	_makefile_print(makefile, "\n\t");
	_makefile_print_escape(makefile, script);
	_makefile_print(makefile, " -P \"%s\"", prefix);
	if(flags != NULL && flags[0] != '\0')
		_makefile_print(makefile, " %s", flags);
	_makefile_print(makefile, " -- \"%s%s\"\n", phony ? "" : "$(OBJDIR)",
			target);
	return 0;
}

static void _script_check(Makefile * makefile, String const * target,
		String const * script)
{
	String * path;

	if((path = _script_path(makefile, script)) == NULL)
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

static int _script_depends(Makefile * makefile, String const * target)
{
	String const * p;

	if((p = _makefile_get_config(makefile, target, "depends")) != NULL
			&& _makefile_expand(makefile, p) != 0)
		return error_print(PROGNAME);
	return 0;
}

static String * _script_path(Makefile * makefile, String const * script)
{
	String * ret;
	String const * directory;
	ssize_t i;
	String * p = NULL;

	if((directory = _makefile_get_config(makefile, NULL, "directory"))
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

static void _script_security(Makefile * makefile, String const * target,
		String const * script)
{
	String * path;

	if((path = _script_path(makefile, script)) == NULL)
	{
		error_print(PROGNAME);
		return;
	}
	error_set_print(PROGNAME, 0, "%s: %s%s%s", target, "The \"", path,
			"\" script is executed while compiling");
	string_delete(path);
}

static int _target_source(Makefile * makefile, String const * target,
		String * source);
static int _objects_target(Makefile * makefile,
		String const * target)
{
	int ret = 0;
	String const * p;
	String * sources;
	String * q;
	size_t i;
	char c;

	if((p = _makefile_get_config(makefile, target, "sources")) == NULL)
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
		ret |= _target_source(makefile, target, sources);
		if(c == '\0')
			break;
		sources += i + 1;
		i = 0;
	}
	string_delete(q);
	return ret;
}

static int _source_depends(Makefile * makefile,
		String const * source);
static int _source_subdir(Makefile * makefile, String * source);
static int _target_source(Makefile * makefile, String const * target,
		String * source)
	/* FIXME check calls to _source_depends() */
{
	int ret = 0;
	String const * extension;
	TargetType tt = TT_UNKNOWN;
	ObjectType ot;
	size_t len;
	String const * p;
	String const * q;

	if((p = _makefile_get_config(makefile, target, "type")) != NULL)
			tt = enum_string(TT_LAST, sTargetType, p);
	if((extension = source_extension(source)) == NULL)
		return 1;
	len = string_length(source) - string_length(extension) - 1;
	source[len] = '\0';
	switch((ot = source_type(extension)))
	{
		case OT_ASM_SOURCE:
		case OT_ASMPP_SOURCE:
			if(tt == TT_OBJECT)
				_makefile_print(makefile, "%s%s", "\n$(OBJDIR)",
						target);
			else
				_makefile_print(makefile, "%s%s%s",
						"\n$(OBJDIR)", source, ".o");
			if(tt == TT_LIBTOOL)
				_makefile_print(makefile, " $(OBJDIR)%s.lo",
						source);
			_makefile_print(makefile, "%s%s%s%s", ": ", source, ".",
					extension);
			source[len] = '.'; /* FIXME ugly */
			_source_depends(makefile, source);
			source[len] = '\0';
			_makefile_print(makefile, "%s", "\n\t");
			if(strchr(source, '/') != NULL)
				ret = _source_subdir(makefile, source);
			if(tt == TT_LIBTOOL)
				_makefile_print(makefile, "%s",
						"$(LIBTOOL) --mode=compile ");
			_makefile_print(makefile, "%s", "$(AS)");
			source[len] = '.'; /* FIXME ugly */
			if(ot == OT_ASMPP_SOURCE)
			{
				_makefile_print(makefile, "%s", " $(");
				_makefile_print_escape_variable(makefile,
						target);
				_makefile_print(makefile, "%s", "_CPPFLAGS)");
				if((p = _makefile_get_config(makefile, source,
								"cppflags"))
						!= NULL)
					_makefile_print(makefile, " %s", p);
			}
			_makefile_print(makefile, "%s", " $(");
			_makefile_print_escape_variable(makefile, target);
			_makefile_print(makefile, "%s", "_ASFLAGS)");
			if((p = _makefile_get_config(makefile, source,
							"asflags")) != NULL)
				_makefile_print(makefile, " %s", p);
			source[len] = '\0'; /* FIXME ugly */
			if(tt == TT_OBJECT)
			{
				_makefile_print(makefile, "%s", " -o $(OBJDIR)");
				_makefile_print_escape(makefile, target);
				_makefile_print(makefile, " ");
				_makefile_print_escape(makefile, source);
				_makefile_print(makefile, ".%s\n", extension);
			}
			else
			{
				_makefile_print(makefile, "%s", " -o $(OBJDIR)");
				_makefile_print_escape(makefile, source);
				_makefile_print(makefile, ".o ");
				_makefile_print_escape(makefile, source);
				_makefile_print(makefile, ".%s\n", extension);
			}
			break;
		case OT_C_SOURCE:
		case OT_OBJC_SOURCE:
			if(tt == TT_OBJECT)
			{
				_makefile_print(makefile, "%s", "\n$(OBJDIR)");
				_makefile_print_escape(makefile, target);
			}
			else
			{
				_makefile_print(makefile, "%s", "\n$(OBJDIR)");
				_makefile_print_escape(makefile, source);
				_makefile_print(makefile, ".o");
			}
			if(tt == TT_LIBTOOL)
			{
				_makefile_print(makefile, " $(OBJDIR)");
				_makefile_print_escape(makefile, source);
				_makefile_print(makefile, "%s", ".lo");
			}
			_makefile_print(makefile, ": ");
			_makefile_print_escape(makefile, source);
			_makefile_print(makefile, ".%s", extension);
			source[len] = '.'; /* FIXME ugly */
			_source_depends(makefile, source);
			_makefile_print(makefile, "%s", "\n\t");
			if(strchr(source, '/') != NULL)
				ret = _source_subdir(makefile, source);
			/* FIXME do both wherever also relevant */
			p = _makefile_get_config(makefile, source, "cppflags");
			q = _makefile_get_config(makefile, source, "cflags");
			source[len] = '\0';
			if(tt == TT_LIBTOOL)
				_makefile_print(makefile, "%s",
						"$(LIBTOOL) --mode=compile ");
			_makefile_print(makefile, "%s", "$(CC)");
			if(p != NULL)
				_makefile_print(makefile, " %s", p);
			_makefile_print(makefile, "%s", " $(");
			_makefile_print_escape_variable(makefile, target);
			_makefile_print(makefile, "%s", "_CFLAGS)");
			if(q != NULL)
			{
				_makefile_print(makefile, " %s", q);
				if(configure_get_os(makefile->configure)
						== HO_GNU_LINUX
					       	&& string_find(q, "-ansi"))
					_makefile_print(makefile, "%s",
							" -D _GNU_SOURCE");
			}
			if(tt == TT_OBJECT)
			{
				_makefile_print(makefile, "%s",
						" -o $(OBJDIR)");
				_makefile_print_escape(makefile, target);
			}
			else
			{
				_makefile_print(makefile, "%s",
						" -o $(OBJDIR)");
				_makefile_print_escape(makefile, source);
				_makefile_print(makefile, ".o");
			}
			_makefile_print(makefile, "%s", " -c ");
			_makefile_print_escape(makefile, source);
			_makefile_print(makefile, ".%s\n", extension);
			break;
		case OT_CXX_SOURCE:
		case OT_OBJCXX_SOURCE:
			if(tt == TT_OBJECT)
			{
				_makefile_print(makefile, "%s", "\n$(OBJDIR)");
				_makefile_print_escape(makefile, target);
			}
			else
			{
				_makefile_print(makefile, "%s", "\n$(OBJDIR)");
				_makefile_print_escape(makefile, source);
				_makefile_print(makefile, "%s", ".o");
			}
			_makefile_print(makefile, "%s%s%s%s", ": ", source, ".",
					extension);
			source[len] = '.'; /* FIXME ugly */
			_source_depends(makefile, source);
			p = _makefile_get_config(makefile, source, "cxxflags");
			source[len] = '\0';
			_makefile_print(makefile, "\n\t");
			if(strchr(source, '/') != NULL)
				ret = _source_subdir(makefile, source);
			if(tt == TT_LIBTOOL)
				_makefile_print(makefile, "%s",
						"$(LIBTOOL) --mode=compile ");
			_makefile_print(makefile, "%s", "$(CXX) $(");
			_makefile_print_escape(makefile, target);
			_makefile_print(makefile, "%s", "_CXXFLAGS)");
			if(p != NULL)
				_makefile_print(makefile, " %s", p);
			if(tt == TT_OBJECT)
			{
				_makefile_print(makefile, "%s", " -o $(OBJDIR)");
				_makefile_print_escape(makefile, target);
			}
			else
			{
				_makefile_print(makefile, "%s", " -o $(OBJDIR)");
				_makefile_print_escape(makefile, source);
				_makefile_print(makefile, ".o");
			}
			_makefile_print(makefile, "%s", " -c ");
			_makefile_print_escape(makefile, source);
			_makefile_print(makefile, ".%s\n", extension);
			break;
		case OT_JAVA_SOURCE:
			_makefile_print(makefile, "%s", "\n$(OBJDIR)");
			_makefile_print_escape(makefile, target);
			_makefile_print(makefile, ": ");
			_makefile_print_escape(makefile, source);
			_makefile_print(makefile, ".%s", extension);
			source[len] = '.'; /* FIXME ugly */
			_source_depends(makefile, source);
			_makefile_print(makefile, "\n\t");
			if(strchr(source, '/') != NULL)
				ret = _source_subdir(makefile, source);
			q = _makefile_get_config(makefile, source, "jflags");
			source[len] = '\0';
			_makefile_print(makefile, "%s", "$(JAVAC) $(");
			_makefile_print_escape_variable(makefile, target);
			_makefile_print(makefile, "%s", "_JFLAGS)");
			if(q != NULL)
				_makefile_print(makefile, " %s", q);
			_makefile_print(makefile, " ");
			_makefile_print_escape(makefile, source);
			_makefile_print(makefile, ".%s\n", extension);
			break;
		case OT_VERILOG_SOURCE:
			if(tt == TT_OBJECT)
			{
				_makefile_print(makefile, "%s", "\n$(OBJDIR)");
				_makefile_print_escape(makefile, target);
			}
			else
			{
				_makefile_print(makefile, "%s", "\n$(OBJDIR)");
				_makefile_print_escape(makefile, source);
				_makefile_print(makefile, ".o");
			}
			_makefile_print(makefile, ": ");
			_makefile_print_escape(makefile, source);
			_makefile_print(makefile, ".%s", extension);
			source[len] = '.'; /* FIXME ugly */
			_source_depends(makefile, source);
			_makefile_print(makefile, "\n\t");
			if(strchr(source, '/') != NULL)
				ret = _source_subdir(makefile, source);
			q = _makefile_get_config(makefile, source, "vflags");
			source[len] = '\0';
			_makefile_print(makefile, "%s", "$(VERILOG) $(");
			_makefile_print_escape_variable(makefile, target);
			_makefile_print(makefile, "%s", "_VFLAGS)");
			if(q != NULL)
				_makefile_print(makefile, " %s", q);
			if(tt == TT_OBJECT)
			{
				_makefile_print(makefile, "%s",
						" -o $(OBJDIR)");
				_makefile_print_escape(makefile, target);
			}
			else
			{
				_makefile_print(makefile, "%s",
						" -o $(OBJDIR)");
				_makefile_print_escape(makefile, source);
				_makefile_print(makefile, ".o");
			}
			_makefile_print(makefile, " ");
			_makefile_print_escape(makefile, source);
			_makefile_print(makefile, "%s%s\n", ".", extension);
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

static int _source_depends(Makefile * makefile,
		String const * target)
{
	String const * p;

	if((p = _makefile_get_config(makefile, target, "depends")) != NULL
			&& _makefile_expand(makefile, p) != 0)
		return error_print(PROGNAME);
	return 0;
}

static int _source_subdir(Makefile * makefile, String * source)
{
	char * p;
	char const * q;

	if((p = strdup(source)) == NULL)
		return 1;
	q = dirname(p);
	_makefile_print(makefile,
			"@[ -d \"%s%s\" ] || $(MKDIR) -- \"%s%s\"\n\t",
			"$(OBJDIR)", q, "$(OBJDIR)", q);
	free(p);
	return 0;
}

static int _clean_targets(Makefile * makefile);
static int _write_clean(Makefile * makefile)
{
	_makefile_target(makefile, "clean", NULL);
	if(_makefile_get_config(makefile, NULL, "subdirs") != NULL)
		_makefile_subdirs(makefile, "clean");
	return _clean_targets(makefile);
}

static int _clean_targets(Makefile * makefile)
{
	String const * prefix;
	String const * flags;
	String const * p;
	String * targets;
	String * q;
	size_t cnt;
	size_t i;
	char c;
	int phony;

	if((p = _makefile_get_config(makefile, NULL, "targets")) == NULL)
		return 0;
	if((targets = string_new(p)) == NULL)
		return 1;
	q = targets;
	/* remove all of the object files */
	for(i = 0, cnt = 0;; i++)
	{
		if(targets[i] != ',' && targets[i] != '\0')
			continue;
		c = targets[i];
		targets[i] = '\0';
		if(_makefile_get_type(makefile, targets) != TT_COMMAND
				&& _makefile_get_type(makefile, targets)
				!= TT_SCRIPT)
		{
			if(cnt++ == 0)
				_makefile_print(makefile, "%s", "\t$(RM) --");
			_makefile_print(makefile, "%s", " $(");
			_makefile_print_escape_variable(makefile, targets);
			_makefile_print(makefile, "%s", "_OBJS)");
		}
		if(c == '\0')
			break;
		targets[i] = c;
		targets += i + 1;
		i = 0;
	}
	if(cnt > 0)
		_makefile_print(makefile, "\n");
	targets = q;
	/* let each scripted target remove the relevant object files */
	for(i = 0;; i++)
	{
		if(targets[i] != ',' && targets[i] != '\0')
			continue;
		c = targets[i];
		targets[i] = '\0';
		if((p = _makefile_get_config(makefile, targets, "type")) != NULL
				&& strcmp(p, "script") == 0
				&& (p = _makefile_get_config(makefile, targets,
						"script")) != NULL)
		{
			if((prefix = _makefile_get_config(makefile, targets,
							"prefix")) == NULL)
				prefix = "$(PREFIX)";
			flags = _makefile_get_config(makefile, targets,
					"flags");
			phony = _makefile_is_phony(makefile, targets);
			_makefile_print(makefile, "\t");
			_makefile_print_escape(makefile, p);
			_makefile_print(makefile, "%s%s%s", " -c -P \"", prefix,
					"\"");
			if(flags != NULL && flags[0] != '\0')
				_makefile_print(makefile, " %s", flags);
			_makefile_print(makefile, "%s%s%s%s", " -- \"",
					phony ? "" : "$(OBJDIR)",
					targets, "\"\n");
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

static int _write_distclean(Makefile * makefile)
{
	String const * subdirs;

	/* only depend on the "clean" target if we do not have subfolders */
	if((subdirs = _makefile_get_config(makefile, NULL, "subdirs")) == NULL)
		_makefile_target(makefile, "distclean", "clean", NULL);
	else
	{
		_makefile_target(makefile, "distclean", NULL);
		_makefile_subdirs(makefile, "distclean");
		_clean_targets(makefile);
	}
	/* FIXME do not erase targets that need be distributed */
	if(_makefile_get_config(makefile, NULL, "targets") != NULL)
		_makefile_remove(makefile, 0, "$(TARGETS)", NULL);
	return 0;
}

static int _dist_subdir(Makefile * makefile, Config * subdir);
static int _write_dist(Makefile * makefile, configArray * ca, int from, int to)
{
	String const * package;
	String const * version;
	Config * p;
	int i;

	package = _makefile_get_config(makefile, NULL, "package");
	version = _makefile_get_config(makefile, NULL, "version");
	if(package == NULL || version == NULL)
		return 0;
	_makefile_target(makefile, "dist", NULL);
	_makefile_remove(makefile, 1, "$(OBJDIR)$(PACKAGE)-$(VERSION)", NULL);
	_makefile_link(makefile, 1, "\"$$PWD\"",
			"$(OBJDIR)$(PACKAGE)-$(VERSION)");
	_makefile_print(makefile, "%s", "\t@cd $(OBJDIR). && $(TAR) -czvf"
			" $(PACKAGE)-$(VERSION)$(TGZEXT) -- \\\n");
	for(i = from + 1; i < to; i++)
	{
		array_get_copy(ca, i, &p);
		_dist_subdir(makefile, p);
	}
	if(from < to)
	{
		array_get_copy(ca, from, &p);
		_dist_subdir(makefile, p);
	}
	else
		return 1;
	_makefile_remove(makefile, 0, "$(OBJDIR)$(PACKAGE)-$(VERSION)", NULL);
	return 0;
}

static int _write_distcheck(Makefile * makefile)
{
	String const * package;
	String const * version;
	const char pretarget[] = "\ndistcheck: dist\n"
		"\t$(TAR) -xzvf $(OBJDIR)$(PACKAGE)-$(VERSION)$(TGZEXT)\n"
		"\t$(MKDIR) -- $(PACKAGE)-$(VERSION)/objdir\n"
		"\t$(MKDIR) -- $(PACKAGE)-$(VERSION)/destdir\n";
	const char target[] = "\tcd \"$(PACKAGE)-$(VERSION)\" && $(MAKE) OBJDIR=\"$$PWD/objdir/\"\n"
		"\tcd \"$(PACKAGE)-$(VERSION)\" && $(MAKE) OBJDIR=\"$$PWD/objdir/\" DESTDIR=\"$$PWD/destdir\" install\n"
		"\tcd \"$(PACKAGE)-$(VERSION)\" && $(MAKE) OBJDIR=\"$$PWD/objdir/\" DESTDIR=\"$$PWD/destdir\" uninstall\n"
		"\tcd \"$(PACKAGE)-$(VERSION)\" && $(MAKE) OBJDIR=\"$$PWD/objdir/\" distclean\n"
		"\tcd \"$(PACKAGE)-$(VERSION)\" && $(MAKE) dist\n";
	const char posttarget[] = "\t$(RM) -r -- $(PACKAGE)-$(VERSION)\n";

	package = _makefile_get_config(makefile, NULL, "package");
	version = _makefile_get_config(makefile, NULL, "version");
	if(package == NULL || version == NULL)
		return 0;
	_makefile_print(makefile, "%s%s%s", pretarget, target, posttarget);
	return 0;
}

static int _dist_subdir_dist(Makefile * makefile, String const * path,
		String const * dist);
static int _dist_subdir(Makefile * makefile, Config * subdir)
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

	path = _makefile_get_config(makefile, NULL, "directory");
	len = (path != NULL) ? string_length(path) : 0;
	if((path = config_get(subdir, NULL, "directory")) == NULL)
		path = "";
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
				_dist_subdir_dist(makefile, path, dist);
			if(c == '\0')
				break;
			targets += i + 1;
			i = 0;
		}
		string_delete(q);
	}
	if((includes = config_get(subdir, NULL, "includes")) != NULL)
		_dist_subdir_dist(makefile, path, includes);
	if((dist = config_get(subdir, NULL, "dist")) != NULL)
		_dist_subdir_dist(makefile, path, dist);
	quote = (strchr(path, ' ') != NULL) ? "\"" : "";
	_makefile_print(makefile, "%s%s%s%s%s%s%s%s", "\t\t", quote,
			"$(PACKAGE)-$(VERSION)/", path,
			(path[0] == '\0') ? "" : "/", PROJECT_CONF, quote,
			(path[0] == '\0') ? "\n" : " \\\n");
	return 0;
}

static int _dist_subdir_dist(Makefile * makefile, String const * path,
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
		_makefile_print(makefile, "%s%s%s%s%s%s%s%s", "\t\t", quote,
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

static int _install_targets(Makefile * makefile);
static int _install_includes(Makefile * makefile);
static int _install_dist(Makefile * makefile);
static int _write_install(Makefile * makefile)
{
	int ret = 0;

	_makefile_target(makefile, "install", "all", NULL);
	if(_makefile_get_config(makefile, NULL, "subdirs") != NULL)
		_makefile_subdirs(makefile, "install");
	ret |= _install_targets(makefile);
	ret |= _install_includes(makefile);
	ret |= _install_dist(makefile);
	return ret;
}

static int _install_target(Makefile * makefile, String const * target);
static int _install_targets(Makefile * makefile)
{
	int ret = 0;
	String const * p;
	String * q;
	String * targets;
	size_t i;
	char c;

	if((p = _makefile_get_config(makefile, NULL, "targets")) == NULL)
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
		if(_makefile_is_enabled(makefile, targets) != 0)
			ret |= _install_target(makefile, targets);
		if(c == '\0')
			break;
		targets += i + 1;
		i = 0;
	}
	string_delete(q);
	return ret;
}

static void _install_target_binary(Makefile * makefile, String const * target);
static void _install_target_command(Makefile * makefile, String const * target);
static int _install_target_library(Makefile * makefile, String const * target);
static void _install_target_libtool(Makefile * makefile, String const * target);
static void _install_target_object(Makefile * makefile, String const * target);
static void _install_target_plugin(Makefile * makefile, String const * target);
static void _install_target_script(Makefile * makefile, String const * target);
static int _install_target(Makefile * makefile, String const * target)
{
	int ret = 0;
	String const * type;
	TargetType tt;

	if((type = _makefile_get_config(makefile, target, "type")) == NULL)
		return 1;
	switch((tt = enum_string(TT_LAST, sTargetType, type)))
	{
		case TT_BINARY:
			_install_target_binary(makefile, target);
			break;
		case TT_COMMAND:
			_install_target_command(makefile, target);
			break;
		case TT_LIBRARY:
			ret = _install_target_library(makefile, target);
			break;
		case TT_LIBTOOL:
			_install_target_libtool(makefile, target);
			break;
		case TT_OBJECT:
			_install_target_object(makefile, target);
			break;
		case TT_PLUGIN:
			_install_target_plugin(makefile, target);
			break;
		case TT_SCRIPT:
			_install_target_script(makefile, target);
			break;
		case TT_UNKNOWN:
			break;
	}
	return ret;
}

static void _install_target_command(Makefile * makefile, String const * target)
{
	String const * path;
	String const * mode;
	mode_t m = 0644;
	String * p;

	if((path = _makefile_get_config(makefile, target, "install")) == NULL)
		return;
	if(_makefile_is_phony(makefile, target))
		return;
	if((mode = _makefile_get_config(makefile, target, "mode")) == NULL
			/* XXX these tests are not sufficient */
			|| mode[0] == '\0'
			|| (m = strtol(mode, &p, 8)) == 0
			|| *p != '\0')
		mode = "0644";
	_makefile_mkdir(makefile, path);
	_makefile_print(makefile, "%s%s%s", "\t$(INSTALL) -m ", mode,
			" $(OBJDIR)");
	_makefile_print_escape(makefile, target);
	_makefile_print(makefile, "%s%s", " $(DESTDIR)", path);
	_makefile_print(makefile, "/");
	_makefile_print_escape(makefile, target);
	_makefile_print(makefile, "\n");
}

static void _install_target_binary(Makefile * makefile, String const * target)
{
	String const * path;

	if((path = _makefile_get_config(makefile, target, "install")) == NULL)
		return;
	_makefile_mkdir(makefile, path);
	_makefile_print(makefile, "%s", "\t$(INSTALL) -m 0755 $(OBJDIR)");
	_makefile_print_escape(makefile, target);
	_makefile_print(makefile, "%s", "$(EXEEXT) $(DESTDIR)");
	_makefile_print_escape(makefile, path);
	_makefile_print(makefile, "/");
	_makefile_print_escape(makefile, target);
	_makefile_print(makefile, "%s\n", "$(EXEEXT)");
}

static int _install_target_library(Makefile * makefile, String const * target)
{
	String const * path;
	String const * p;
	String * soname;
	HostOS os;

	if((path = _makefile_get_config(makefile, target, "install")) == NULL)
		return 0;
	_makefile_mkdir(makefile, path);
	if(configure_can_library_static(makefile->configure))
		/* install the static library */
		_makefile_print(makefile, "%s%s%s%s/%s%s",
				"\t$(INSTALL) -m 0644 $(OBJDIR)", target,
				".a $(DESTDIR)", path, target, ".a\n");
	os = configure_get_os(makefile->configure);
	if((p = _makefile_get_config(makefile, target, "soname")) != NULL)
		soname = string_new(p);
	else if(os == HO_MACOSX)
		/* versioning is different on MacOS X */
		soname = string_new_append(target, ".0.0$(SOEXT)", NULL);
	else if(os == HO_WIN32)
		/* and on Windows */
		soname = string_new_append(target, "$(SOEXT)", NULL);
	else
		soname = string_new_append(target, "$(SOEXT)", ".0", NULL);
	if(soname == NULL)
		return 1;
	/* install the shared library */
	if(os == HO_MACOSX)
	{
		_makefile_print(makefile, "%s%s%s%s/%s%s",
				"\t$(INSTALL) -m 0755 $(OBJDIR)", soname,
				" $(DESTDIR)", path, soname, "\n");
		_makefile_print(makefile, "%s%s%s%s/%s%s", "\t$(LN) -s -- ",
				soname, " $(DESTDIR)", path, target,
				".0$(SOEXT)\n");
		_makefile_print(makefile, "%s%s%s%s/%s%s", "\t$(LN) -s -- ",
				soname, " $(DESTDIR)", path, target,
				"$(SOEXT)\n");
	}
	else if(os == HO_WIN32)
		_makefile_print(makefile, "%s%s%s%s%s%s%s",
				"\t$(INSTALL) -m 0755 $(OBJDIR)", soname,
				" $(DESTDIR)", path, "/", soname, "\n");
	else
	{
		_makefile_print(makefile, "%s%s%s%s/%s%s",
				"\t$(INSTALL) -m 0755 $(OBJDIR)", soname,
				".0 $(DESTDIR)", path, soname, ".0\n");
		_makefile_print(makefile, "%s%s%s%s/%s%s", "\t$(LN) -s -- ",
				soname, ".0 $(DESTDIR)", path, soname, "\n");
		_makefile_print(makefile, "%s%s%s%s/%s%s", "\t$(LN) -s -- ",
				soname, ".0 $(DESTDIR)", path, target,
				"$(SOEXT)\n");
	}
	string_delete(soname);
	return 0;
}

static void _install_target_libtool(Makefile * makefile, String const * target)
{
	String const * path;

	if((path = _makefile_get_config(makefile, target, "install")) == NULL)
		return;
	_makefile_mkdir(makefile, path);
	_makefile_print(makefile, "%s", "\t$(LIBTOOL) --mode=install $(INSTALL)"
			" -m 0755 $(OBJDIR)");
	_makefile_print_escape(makefile, target);
	_makefile_print(makefile, ".la $(DESTDIR)");
	_makefile_print_escape(makefile, path);
	_makefile_print(makefile, "/");
	_makefile_print_escape(makefile, target);
	_makefile_print(makefile, ".la\n");
	_makefile_print(makefile, "%s", "\t$(LIBTOOL) --mode=finish $(DESTDIR)");
	_makefile_print_escape(makefile, path);
	_makefile_print(makefile, "\n");
}

static void _install_target_object(Makefile * makefile, String const * target)
{
	String const * path;

	if((path = _makefile_get_config(makefile, target, "install")) == NULL)
		return;
	_makefile_mkdir(makefile, path);
	_makefile_print(makefile, "%s%s%s%s/%s\n",
			"\t$(INSTALL) -m 0644 $(OBJDIR)",
			target, " $(DESTDIR)", path, target);
}

static void _install_target_plugin(Makefile * makefile, String const * target)
{
	String const * path;
	String const * mode;
	mode_t m = 0755;
	String * p;

	if((path = _makefile_get_config(makefile, target, "install")) == NULL)
		return;
	if((mode = _makefile_get_config(makefile, target, "mode")) == NULL
			/* XXX these tests are not sufficient */
			|| mode[0] == '\0'
			|| (m = strtol(mode, &p, 8)) == 0
			|| *p != '\0')
		mode = "0755";
	_makefile_mkdir(makefile, path);
	_makefile_print(makefile, "%s%04o%s", "\t$(INSTALL) -m ", m,
			" $(OBJDIR)");
	_makefile_print_escape(makefile, target);
	_makefile_print(makefile, "%s", "$(SOEXT) $(DESTDIR)");
	_makefile_print_escape(makefile, path);
	_makefile_print(makefile, "/");
	_makefile_print_escape(makefile, target);
	_makefile_print(makefile, "%s", "$(SOEXT)\n");
}

static void _install_target_script(Makefile * makefile, String const * target)
{
	String const * path;
	String const * script;
	String const * flags;
	int phony;

	if((path = _makefile_get_config(makefile, target, "install")) == NULL)
		return;
	if((script = _makefile_get_config(makefile, target, "script")) == NULL)
		return;
	flags = _makefile_get_config(makefile, target, "flags");
	phony = _makefile_is_phony(makefile, target);
	_makefile_print(makefile, "\t");
	_makefile_print_escape(makefile, script);
	_makefile_print(makefile, "%s%s%s", " -P \"$(DESTDIR)",
			(path[0] != '\0') ? path : "$(PREFIX)", "\" -i");
	if(flags != NULL && flags[0] != '\0')
		_makefile_print(makefile, " %s", flags);
	_makefile_print(makefile, "%s%s%s%s", " -- \"",
			phony ? "" : "$(OBJDIR)", target, "\"\n");
}

static int _install_include(Makefile * makefile, String const * include);
static int _install_includes(Makefile * makefile)
{
	int ret = 0;
	String const * p;
	String * includes;
	String * q;
	size_t i;
	char c;

	if((p = _makefile_get_config(makefile, NULL, "includes")) == NULL)
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
		ret |= _install_include(makefile, includes);
		if(c == '\0')
			break;
		includes += i + 1;
		i = 0;
	}
	string_delete(q);
	return ret;
}

static int _install_include(Makefile * makefile, String const * include)
{
	char const * install;
	ssize_t i;
	String * p = NULL;
	String * directory;

	if((install = _makefile_get_config(makefile, include, "install"))
			== NULL)
	{
		install = "$(INCLUDEDIR)";
		if((i = string_rindex(include, "/")) >= 0)
			if((p = string_new_length(include, i)) == NULL)
				return 2;
	}
	if(p != NULL)
	{
		directory = string_new_append(install, "/", p, NULL);
		string_delete(p);
	}
	else
		directory = string_new(install);
	if(directory == NULL)
		return 2;
	_makefile_mkdir(makefile, directory);
	string_delete(directory);
	_makefile_print(makefile, "%s", "\t$(INSTALL) -m 0644 ");
	_makefile_print_escape(makefile, include);
	_makefile_print(makefile, "%s", " $(DESTDIR)");
	_makefile_print_escape(makefile, install);
	_makefile_print(makefile, "/");
	_makefile_print_escape(makefile, include);
	_makefile_print(makefile, "\n");
	return 0;
}

static int _dist_check(Makefile * makefile, char const * target,
		char const * mode);
static int _dist_install(Makefile * makefile, char const * directory,
		char const * mode, char const * filename);
static int _install_dist(Makefile * makefile)
{
	int ret = 0;
	String const * p;
	String * dist;
	String * q;
	size_t i;
	char c;
	String const * d;
	String const * mode;

	if((p = _makefile_get_config(makefile, NULL, "dist")) == NULL)
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
		if((mode = _makefile_get_config(makefile, dist, "mode"))
				== NULL)
			mode = "0644";
		ret |= _dist_check(makefile, dist, mode);
		if((d = _makefile_get_config(makefile, dist, "install"))
				!= NULL)
			_dist_install(makefile, d, mode, dist);
		if(c == '\0')
			break;
		dist += i + 1;
		i = 0;
	}
	string_delete(q);
	return ret;
}

static int _dist_check(Makefile * makefile, char const * target,
		char const * mode)
{
	char * p;
	mode_t m;

	m = strtol(mode, &p, 8);
	if(mode[0] == '\0' || *p != '\0')
		return error_set_print(PROGNAME, 1, "%s: %s%s%s", target,
				"Invalid permissions \"", mode, "\"");
	if(_makefile_is_flag_set(makefile, PREFS_S) && (m & S_ISUID))
		error_set_print(PROGNAME, 0, "%s: %s", target,
				"Installed as a SUID file");
	if(_makefile_is_flag_set(makefile, PREFS_S) && (m & S_ISUID))
		error_set_print(PROGNAME, 0, "%s: %s", target,
				"Installed as a SGID file");
	if(_makefile_is_flag_set(makefile, PREFS_S)
			&& (m & (S_IXUSR | S_IXGRP | S_IXOTH)))
		error_set_print(PROGNAME, 0, "%s: %s", target,
				"Installed as an executable file");
	if(_makefile_is_flag_set(makefile, PREFS_S) && (m & S_IWGRP))
		error_set_print(PROGNAME, 0, "%s: %s", target,
				"Installed as a group-writable file");
	if(_makefile_is_flag_set(makefile, PREFS_S) && (m & S_IWOTH))
		error_set_print(PROGNAME, 0, "%s: %s", target,
				"Installed as a writable file");
	return 0;
}

static int _dist_install(Makefile * makefile, char const * directory,
		char const * mode, char const * filename)
{
	String * p;
	char const * q;

	if(strchr(filename, '/') != NULL)
	{
		if((p = string_new(filename)) == NULL)
			return -1;
		q = dirname(p);
		/* TODO keep track of the directories created */
		_makefile_print(makefile, "%s%s/%s\n", "\t$(MKDIR) $(DESTDIR)",
				directory, q);
		string_delete(p);
	}
	else
		_makefile_mkdir(makefile, directory);
	_makefile_print(makefile, "%s", "\t$(INSTALL) -m ");
	_makefile_print_escape(makefile, mode);
	_makefile_print(makefile, " ");
	_makefile_print_escape(makefile, filename);
	_makefile_print(makefile, " $(DESTDIR)");
	_makefile_print_escape(makefile, directory);
	_makefile_print(makefile, "/");
	_makefile_print_escape(makefile, filename);
	_makefile_print(makefile, "\n");
	return 0;
}

static int _write_phony_targets(Makefile * makefile);
static int _write_phony(Makefile * makefile, char const ** targets)
{
	size_t i;

	_makefile_print(makefile, "%s:", "\n.PHONY");
	for(i = 0; targets[i] != NULL; i++)
		_makefile_print(makefile, " %s", targets[i]);
	if(_write_phony_targets(makefile) != 0)
		return 1;
	_makefile_print(makefile, "\n");
	return 0;
}

static int _write_phony_targets(Makefile * makefile)
{
	String const * p;
	String * prints;
	size_t i;
	char c;
	String const * type;

	if((p = _makefile_get_config(makefile, NULL, "targets")) == NULL)
		return 0;
	if((prints = string_new(p)) == NULL)
		return 1;
	for(i = 0;; i++)
	{
		if(prints[i] != ',' && prints[i] != '\0')
			continue;
		c = prints[i];
		prints[i] = '\0';
		if((type = _makefile_get_config(makefile, prints, "type"))
				!= NULL)
			switch(enum_string(TT_LAST, sTargetType, type))
			{
				case TT_COMMAND:
				case TT_SCRIPT:
					if(_makefile_is_phony(makefile, prints))
					{
						_makefile_print(makefile, " ");
						_makefile_print_escape(makefile,
								prints);
					}
					break;
			}
		if(c == '\0')
			break;
		prints += i + 1;
		i = 0;
	}
	return 0;
}

static int _uninstall_target(Makefile * makefile,
		String const * target);
static int _uninstall_include(Makefile * makefile,
		String const * include);
static int _uninstall_dist(Makefile * makefile,
		String const * dist);
static int _write_uninstall(Makefile * makefile)
{
	int ret = 0;
	String const * p;
	String * targets;
	String * q;
	String * includes;
	String * dist;
	size_t i;
	char c;

	_makefile_target(makefile, "uninstall", NULL);
	if(_makefile_get_config(makefile, NULL, "subdirs") != NULL)
		_makefile_subdirs(makefile, "uninstall");
	if((p = _makefile_get_config(makefile, NULL, "targets")) != NULL)
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
			if(_makefile_is_enabled(makefile, targets) != 0)
				ret = _uninstall_target(makefile, targets);
			if(c == '\0')
				break;
			targets += i + 1;
			i = 0;
		}
		string_delete(q);
	}
	if((p = _makefile_get_config(makefile, NULL, "includes")) != NULL)
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
			ret = _uninstall_include(makefile, includes);
			if(c == '\0')
				break;
			includes += i + 1;
			i = 0;
		}
		string_delete(q);
	}
	if((p = _makefile_get_config(makefile, NULL, "dist")) != NULL)
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
			ret = _uninstall_dist(makefile, dist);
			if(c == '\0')
				break;
			dist += i + 1;
			i = 0;
		}
		string_delete(q);
	}
	return ret;
}

static int _uninstall_target_library(Makefile * makefile,
		String const * target, String const * path);
static void _uninstall_target_script(Makefile * makefile,
		String const * target, String const * path);
static int _uninstall_target(Makefile * makefile,
		String const * target)
{
	String const * type;
	String const * path;
	TargetType tt;
	const String rm_destdir[] = "$(RM) -- $(DESTDIR)";

	if((type = _makefile_get_config(makefile, target, "type")) == NULL)
		return 1;
	if((path = _makefile_get_config(makefile, target, "install")) == NULL)
		return 0;
	tt = enum_string(TT_LAST, sTargetType, type);
	switch(tt)
	{
		case TT_BINARY:
			_makefile_print(makefile, "\t%s", rm_destdir);
			_makefile_print_escape(makefile, path);
			_makefile_print(makefile, "/");
			_makefile_print_escape(makefile, target);
			_makefile_print(makefile, "$(EXEEXT)\n");
			break;
		case TT_COMMAND:
			_makefile_print(makefile, "\t%s", rm_destdir);
			_makefile_print_escape(makefile, path);
			_makefile_print(makefile, "/");
			_makefile_print_escape(makefile, target);
			_makefile_print(makefile, "\n");
			break;
		case TT_LIBRARY:
			if(_uninstall_target_library(makefile, target,
						path) != 0)
				return 1;
			break;
		case TT_LIBTOOL:
			_makefile_print(makefile, "\t%s%s", "$(LIBTOOL)"
					" --mode=uninstall ", rm_destdir);
			_makefile_print_escape(makefile, path);
			_makefile_print(makefile, "/");
			_makefile_print_escape(makefile, target);
			_makefile_print(makefile, ".la\n");
			break;
		case TT_OBJECT:
			_makefile_print(makefile, "\t%s", rm_destdir);
			_makefile_print_escape(makefile, path);
			_makefile_print(makefile, "/");
			_makefile_print_escape(makefile, target);
			_makefile_print(makefile, "\n");
			break;
		case TT_PLUGIN:
			_makefile_print(makefile, "\t%s", rm_destdir);
			_makefile_print_escape(makefile, path);
			_makefile_print(makefile, "/");
			_makefile_print_escape(makefile, target);
			_makefile_print(makefile, "$(SOEXT)\n");
			break;
		case TT_SCRIPT:
			_uninstall_target_script(makefile, target, path);
			break;
		case TT_UNKNOWN:
			break;
	}
	return 0;
}

static int _uninstall_target_library(Makefile * makefile,
		String const * target, String const * path)
{
	String * soname;
	String const * p;
	const String format[] = "\t%s%s/%s%s%s%s";
	const String rm_destdir[] = "$(RM) -- $(DESTDIR)";
	HostOS os;

	if(configure_can_library_static(makefile->configure))
		/* uninstall the static library */
		_makefile_print(makefile, format, rm_destdir, path, target,
				".a\n", "", "");
	os = configure_get_os(makefile->configure);
	if((p = _makefile_get_config(makefile, target, "soname")) != NULL)
		soname = string_new(p);
	else if(os == HO_MACOSX)
		/* versioning is different on MacOS X */
		soname = string_new_append(target, ".0.0$(SOEXT)", NULL);
	else if(os == HO_WIN32)
		/* and on Windows */
		soname = string_new_append(target, "$(SOEXT)", NULL);
	else
		soname = string_new_append(target, "$(SOEXT).0", NULL);
	if(soname == NULL)
		return 1;
	/* uninstall the shared library */
	if(os == HO_MACOSX)
	{
		_makefile_print(makefile, format, rm_destdir, path, soname,
				"\n", "", "");
		_makefile_print(makefile, format, rm_destdir, path, target,
				".0", "$(SOEXT)", "\n");
	}
	else if(os != HO_WIN32)
	{
		_makefile_print(makefile, format, rm_destdir, path, soname,
				".0\n", "", "");
		_makefile_print(makefile, format, rm_destdir, path, soname,
				"\n", "", "");
	}
	_makefile_print(makefile, format, rm_destdir, path, target, "$(SOEXT)",
			"\n", "");
	string_delete(soname);
	return 0;
}

static void _uninstall_target_script(Makefile * makefile,
		String const * target, String const * path)
{
	String const * script;
	String const * flags;

	if((script = _makefile_get_config(makefile, target, "script")) == NULL)
		return;
	flags = _makefile_get_config(makefile, target, "flags");
	_makefile_print(makefile, "\t");
	_makefile_print_escape(makefile, script);
	_makefile_print(makefile, "%s%s%s", " -P \"$(DESTDIR)",
			(path[0] != '\0') ? path : "$(PREFIX)", "\" -u");
	if(flags != NULL && flags[0] != '\0')
		_makefile_print(makefile, " %s", flags);
	_makefile_print(makefile, "%s%s%s", " -- \"", target, "\"\n");
}

static int _uninstall_include(Makefile * makefile,
		String const * include)
{
	String const * install;

	if((install = _makefile_get_config(makefile, include, "install"))
			== NULL)
		install = "$(INCLUDEDIR)";
	_makefile_print(makefile, "%s", "\t$(RM) -- $(DESTDIR)");
	_makefile_print_escape(makefile, install);
	_makefile_print(makefile, "/");
	_makefile_print_escape(makefile, include);
	_makefile_print(makefile, "\n");
	return 0;
}

static int _uninstall_dist(Makefile * makefile,
		String const * dist)
{
	String const * install;

	if((install = _makefile_get_config(makefile, dist, "install")) == NULL)
		return 0;
	_makefile_print(makefile, "%s", "\t$(RM) -- $(DESTDIR)");
	_makefile_print_escape(makefile, install);
	_makefile_print(makefile, "/");
	_makefile_print_escape(makefile, dist);
	_makefile_print(makefile, "\n");
	return 0;
}


/* accessors */
/* makefile_get_config */
static String const * _makefile_get_config(Makefile * makefile,
		String const * section, String const * variable)
{
	return configure_get_config(makefile->configure, section, variable);
}


/* makefile_get_type */
static TargetType _makefile_get_type(Makefile * makefile,
		String const * target)
{
	String const * type;

	if((type = _makefile_get_config(makefile, target, "type")) == NULL)
		return TT_UNKNOWN;
	return enum_string(TT_LAST, sTargetType, type);
}


/* makefile_is_enabled */
static int _makefile_is_enabled(Makefile * makefile, char const * target)
{
	String const * p;

	if((p = _makefile_get_config(makefile, target, "enabled")) != NULL
			&& strtol(p, NULL, 10) == 0)
		return 0;
	return 1;
}


/* makefile_is_flag_set */
static unsigned int _makefile_is_flag_set(Makefile * makefile,
		unsigned int flag)
{
	return configure_is_flag_set(makefile->configure, flag);
}


/* makefile_is_phony */
static int _makefile_is_phony(Makefile * makefile, char const * target)
{
	String const * p;

	if((p = _makefile_get_config(makefile, target, "phony")) != NULL
			&& strtol(p, NULL, 10) == 1)
		return 1;
	return 0;
}


/* useful */
/* makefile_expand */
static int _makefile_expand(Makefile * makefile, char const * field)
{
	int res;
	char c;

	if(field == NULL)
		return -1;
	res = _makefile_print(makefile, " ");
	while((c = *(field++)) != '\0')
	{
		if(c == ' ' || c == '\t')
			res |= _makefile_print(makefile, "\\");
		else if(c == ',')
			c = ' ';
		res |= _makefile_print(makefile, "%c", c);
	}
	return (res >= 0) ? 0 : -1;
}


/* makefile_link */
static int _makefile_link(Makefile * makefile, int symlink, char const * link,
		char const * path)
{
	_makefile_print(makefile, "\t$(LN)%s -- %s %s\n", symlink ? " -s" : "",
			link, path);
	return 0;
}


/* makefile_output_extension */
static int _makefile_output_extension(Makefile * makefile,
		String const * extension)
{
	int ret;
	String const * value;
	String * upper;

	if((upper = string_new_append(extension, "EXT", NULL)) == NULL)
		return -1;
	string_toupper(upper);
	value = configure_get_extension(makefile->configure, extension);
	ret = _makefile_output_variable(makefile, upper, value);
	string_delete(upper);
	return ret;
}


/* makefile_output_path */
static int _makefile_output_path(Makefile * makefile, String const * path)
{
	int ret;
	String const * value;
	String * upper;
	String * p;

	if((upper = string_new(path)) == NULL)
		return -1;
	string_toupper(upper);
	value = configure_get_path(makefile->configure, path);
	if(value != NULL && value[0] != '\0')
	{
		if(value[0] != '/')
		{
			p = string_new_append("$(PREFIX)/", value, NULL);
			ret = _makefile_output_variable(makefile, upper, p);
			string_delete(p);
		}
		else
			ret = _makefile_output_variable(makefile, upper, value);
	}
	else
		ret = _makefile_output_variable(makefile, upper, value);
	string_delete(upper);
	return ret;
}


/* makefile_output_program */
static int _makefile_output_program(Makefile * makefile, String const * name,
		unsigned int override)
{
	int ret;
	String const * value;
	String * upper;

	if((upper = string_new(name)) == NULL)
		return -1;
	string_toupper(upper);
	if(override)
		value = _makefile_get_config(makefile, NULL, name);
	else
		value = NULL;
	if(value == NULL)
		value = configure_get_program(makefile->configure, name);
	ret = _makefile_output_variable(makefile, upper, value);
	string_delete(upper);
	return ret;
}


/* makefile_output_variable */
static int _makefile_output_variable(Makefile * makefile, String const * name,
		String const * value)
{
	int res;
	size_t len;

	if(makefile->fp == NULL)
		return 0;
	if(name == NULL || (len = strlen(name)) == 0)
		return -1;
	if(value == NULL)
		value = "";
	res = _makefile_print(makefile, "%s%s%s%s\n", name,
			(len >= 8) ? "" : "\t",
			(strlen(value) > 0) ? "= " : "=", value);
	return (res >= 0) ? 0 : -1;
}


/* makefile_mkdir */
static int _makefile_mkdir(Makefile * makefile, char const * directory)
{
	/* FIXME keep track of the directories created */
	if(_makefile_print(makefile, "%s", "\t$(MKDIR) $(DESTDIR)") < 0
			|| _makefile_print_escape(makefile, directory) < 0
			|| _makefile_print(makefile, "\n") < 0)
		return -1;
	return 0;
}


/* makefile_print */
static int _makefile_print(Makefile * makefile, char const * format, ...)
{
	int ret;
	va_list ap;

	va_start(ap, format);
	if(makefile->fp == NULL)
		ret = vsnprintf(NULL, 0, format, ap);
	else
		ret = vfprintf(makefile->fp, format, ap);
	va_end(ap);
	return ret;
}


/* makefile_print_escape */
static int _makefile_print_escape(Makefile * makefile, char const * str)
{
	if(str == NULL)
		return -1;
	if(makefile->fp == NULL)
		return 0;
	while(*str != '\0')
	{
		if(*str == ' ' || *str == '\t')
			if(fputc('\\', makefile->fp) == EOF)
				return -1;
		if(fputc(*(str++), makefile->fp) == EOF)
			return -1;
	}
	return 0;
}


/* makefile_print_escape_variable */
static int _makefile_print_escape_variable(Makefile * makefile,
		char const * str)
{
	char c;

	if(str == NULL)
		return -1;
	if(makefile->fp == NULL)
		return 0;
	while((c = *(str++)) != '\0')
	{
		if(c == ' ' || c == '\t')
			c = '_';
		if(fputc(c, makefile->fp) == EOF)
			return -1;
	}
	return 0;
}


/* makefile_remove */
static int _makefile_remove(Makefile * makefile, int recursive, ...)
{
	va_list ap;
	char const * sep = " -- ";
	char const * p;

	_makefile_print(makefile, "\t$(RM)%s", recursive ? " -r" : "");
	va_start(ap, recursive);
	while((p = va_arg(ap, char const * )) != NULL)
	{
		_makefile_print(makefile, "%s%s", sep, p);
		sep = " ";
	}
	va_end(ap);
	_makefile_print(makefile, "\n");
	return 0;
}


/* makefile_subdirs */
static int _makefile_subdirs(Makefile * makefile, char const * target)
{
	if(target != NULL)
		_makefile_print(makefile,
				"\t@for i in $(SUBDIRS); do (cd \"$$i\" && \\\n"
				"\t\tif [ -n \"$(OBJDIR)\" ]; then \\\n"
				"\t\t$(MAKE) OBJDIR=\"$(OBJDIR)$$i/\" %s; \\\n"
				"\t\telse $(MAKE) %s; fi) || exit; done\n",
				target, target);
	else
		_makefile_print(makefile, "%s",
				"\t@for i in $(SUBDIRS); do (cd \"$$i\" && \\\n"
				"\t\tif [ -n \"$(OBJDIR)\" ]; then \\\n"
				"\t\t([ -d \"$(OBJDIR)$$i\" ]"
				" || $(MKDIR) -- \"$(OBJDIR)$$i\") && \\\n"
				"\t\t$(MAKE) OBJDIR=\"$(OBJDIR)$$i/\"; \\\n"
				"\t\telse $(MAKE); fi) || exit; done\n");
	return 0;
}


/* makefile_target */
static int _makefile_target(Makefile * makefile, char const * target, ...)
{
	va_list ap;
	char const * sep = " ";
	char const * p;

	if(target == NULL)
		return -1;
	_makefile_print(makefile, "\n%s:", target);
	va_start(ap, target);
	while((p = va_arg(ap, char const *)) != NULL)
		_makefile_print(makefile, "%s%s", sep, p);
	va_end(ap);
	_makefile_print(makefile, "\n");
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
	_makefile_print(makefile, "\n%s:", target);
	if(depends != NULL)
		for(p = depends; *p != NULL; p++)
			_makefile_print(makefile, " %s", *p);
	_makefile_print(makefile, "\n");
	return 0;
}
#endif
