/* $Id$ */
/* Copyright (c) 2006-2014 Pierre Pronchery <khorben@defora.org> */
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
/* TODO:
 * - only check the PREFS_n flags inside a wrapper around fputs()/fprintf() */



#include <System.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include "settings.h"
#include "configure.h"
#include "../config.h"

#ifndef PROGNAME
# define PROGNAME PACKAGE
#endif


/* prototypes */
static int _makefile_link(FILE * fp, int symlink, char const * link,
		char const * path);
static int _makefile_output_variable(FILE * fp, char const * name,
		char const * value);
static int _makefile_remove(FILE * fp, int recursive, ...);
static int _makefile_subdirs(FILE * fp, char const * target);
static int _makefile_target(FILE * fp, char const * target, ...);
static int _makefile_targetv(FILE * fp, char const * target,
		char const ** depends);


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

	makefile = string_new(directory);
	string_append(&makefile, "/"); /* FIXME check for errors */
	string_append(&makefile, MAKEFILE);
	if(!(configure->prefs->flags & PREFS_n)
			&& (fp = fopen(makefile, "w")) == NULL)
		ret = configure_error(makefile, 1);
	else
	{
		if(configure->prefs->flags & PREFS_v)
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
static int _write_uninstall(Configure * configure, FILE * fp);
static int _makefile_write(Configure * configure, FILE * fp, configArray * ca,
	       	int from, int to)
{
	Config * config = configure->config;
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
	if(!(configure->prefs->flags & PREFS_n))
	{
		if(config_get(config, NULL, "subdirs") != NULL)
			depends[i++] = "subdirs";
		depends[i++] = "clean";
		depends[i++] = "distclean";
		if(config_get(config, NULL, "package") != NULL
				&& config_get(config, NULL, "version") != NULL)
		{
			depends[i++] = "dist";
			depends[i++] = "distcheck";
		}
		depends[i++] = "install";
		depends[i++] = "uninstall";
		depends[i++] = NULL;
		_makefile_targetv(fp, ".PHONY", depends);
	}
	return 0;
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
static int _write_variables(Configure * configure, FILE * fp)
{
	int ret = 0;
	String const * directory;
	
	directory = config_get(configure->config, NULL, "directory");
	ret |= _variables_package(configure, fp, directory);
	ret |= _variables_print(configure, fp, "subdirs", "SUBDIRS");
	ret |= _variables_dist(configure, fp);
	ret |= _variables_targets(configure, fp);
	ret |= _variables_executables(configure, fp);
	ret |= _variables_includes(configure, fp);
	if(!(configure->prefs->flags & PREFS_n))
		fputc('\n', fp);
	return ret;
}

static int _variables_package(Configure * configure, FILE * fp,
		String const * directory)
{
	String const * package;
	String const * version;
	String const * p;

	if((package = config_get(configure->config, NULL, "package")) == NULL)
		return 0;
	if(configure->prefs->flags & PREFS_v)
		printf("%s%s", "Package: ", package);
	if((version = config_get(configure->config, NULL, "version")) == NULL)
	{
		if(configure->prefs->flags & PREFS_v)
			fputc('\n', stdout);
		fprintf(stderr, "%s%s%s", PROGNAME ": ", directory,
				": \"package\" needs \"version\"\n");
		return 1;
	}
	if(configure->prefs->flags & PREFS_v)
		printf(" %s\n", version);
	_makefile_output_variable(fp, "PACKAGE", package);
	_makefile_output_variable(fp, "VERSION", version);
	if((p = config_get(configure->config, NULL, "config")) != NULL)
		return settings(configure->prefs, configure->config, directory,
			       	package, version);
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

	if(configure->prefs->flags & PREFS_n)
		return 0;
	if((p = config_get(configure->config, NULL, input)) == NULL)
		return 0;
	if((prints = string_new(p)) == NULL)
		return 1;
	q = prints;
	fprintf(fp, "%s%s", output, "\t=");
	for(i = 0;; i++)
	{
		if(prints[i] != ',' && prints[i] != '\0')
			continue;
		c = prints[i];
		prints[i] = '\0';
		if(strchr(prints, ' ') != NULL)
			fprintf(fp, " \"%s\"", prints);
		else
			fprintf(fp, " %s", prints);
		if(c == '\0')
			break;
		prints += i + 1;
		i = 0;
	}
	fputc('\n', fp);
	string_delete(q);
	return 0;
}

static int _variables_dist(Configure * configure, FILE * fp)
{
	String const * p;
	String * dist;
	String * q;
	size_t i;
	char c;

	if(configure->prefs->flags & PREFS_n)
		return 0;
	if((p = config_get(configure->config, NULL, "dist")) == NULL)
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
		if(config_get(configure->config, dist, "install") != NULL)
		{
			/* FIXME may still need to be output */
			if(config_get(configure->config, NULL, "targets")
					== NULL)
			{
				_makefile_output_variable(fp, "OBJDIR", "");
				_makefile_output_variable(fp, "PREFIX",
						configure->prefs->prefix);
				_makefile_output_variable(fp, "DESTDIR",
						configure->prefs->destdir);
			}
			_makefile_output_variable(fp, "MKDIR",
					configure->programs.mkdir);
			_makefile_output_variable(fp, "INSTALL",
					configure->programs.install);
			_makefile_output_variable(fp, "RM",
					configure->programs.rm);
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
	String const * soext;
	String * q;
	size_t i;
	char c;
	String const * type;

	if(configure->prefs->flags & PREFS_n)
		return 0;
	if((p = config_get(configure->config, NULL, "targets")) == NULL)
		return 0;
	if((prints = string_new(p)) == NULL)
		return 1;
	soext = configure_get_soext(configure);
	q = prints;
	fputs("TARGETS\t=", fp);
	for(i = 0;; i++)
	{
		if(prints[i] != ',' && prints[i] != '\0')
			continue;
		c = prints[i];
		prints[i] = '\0';
		if((type = config_get(configure->config, prints, "type"))
			       	== NULL)
			fprintf(fp, " %s", prints);
		else
			switch(enum_string(TT_LAST, sTargetType, type))
			{
				case TT_BINARY:
					fprintf(fp, " $(OBJDIR)%s", prints);
					if(configure->os == HO_WIN32)
						fputs("$(EXEEXT)", fp);
					break;
				case TT_OBJECT:
				case TT_SCRIPT:
				case TT_UNKNOWN:
					fprintf(fp, " $(OBJDIR)%s", prints);
					break;
				case TT_LIBRARY:
					ret |= _variables_targets_library(
							configure, fp, prints);
					break;
				case TT_LIBTOOL:
					fprintf(fp, " $(OBJDIR)%s%s", prints,
							".la");
					break;
				case TT_PLUGIN:
					fprintf(fp, " $(OBJDIR)%s%s", prints, soext);
					break;
			}
		if(c == '\0')
			break;
		prints += i + 1;
		i = 0;
	}
	fputc('\n', fp);
	string_delete(q);
	return ret;
}

static int _variables_targets_library(Configure * configure, FILE * fp,
		char const * target)
{
	String const * soext;
	String * soname;
	String const * p;

	soext = configure_get_soext(configure);
	if((p = config_get(configure->config, target, "soname")) != NULL)
		soname = string_new(p);
	else if(configure->os == HO_MACOSX)
		/* versioning is different on MacOS X */
		soname = string_new_append(target, ".0.0", soext, NULL);
	else if(configure->os == HO_WIN32)
		/* and on Windows */
		soname = string_new_append(target, soext, NULL);
	else
		soname = string_new_append(target, soext, ".0", NULL);
	if(soname == NULL)
		return 1;
	if(configure_can_library_static(configure))
		/* generate a static library */
		fprintf(fp, "%s%s%s", " $(OBJDIR)", target, ".a");
	if(configure->os == HO_MACOSX)
		fprintf(fp, "%s%s%s%s%s%s%s%s%s", " $(OBJDIR)", soname,
				" $(OBJDIR)", target, ".0", soext, " $(OBJDIR)",
				target, soext);
	else if(configure->os == HO_WIN32)
		fprintf(fp, "%s%s", " $(OBJDIR)", soname);
	else
		fprintf(fp, "%s%s%s%s%s%s%s", " $(OBJDIR)", soname,
				".0 $(OBJDIR)", soname, " $(OBJDIR)", target,
				soext);
	string_delete(soname);
	return 0;
}

static void _executables_variables(Configure * configure, FILE * fp,
	       	String const * target, char * done);
static int _variables_executables(Configure * configure, FILE * fp)
{
	char done[TT_LAST]; /* FIXME even better if'd be variable by variable */
	String const * targets;
	String const * includes;
	String const * package;
	String * p;
	String * q;
	size_t i;
	char c;

	if(configure->prefs->flags & PREFS_n)
		return 0;
	memset(&done, 0, sizeof(done));
	targets = config_get(configure->config, NULL, "targets");
	includes = config_get(configure->config, NULL, "includes");
	package = config_get(configure->config, NULL, "package");
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
		_makefile_output_variable(fp, "PREFIX",
				configure->prefs->prefix);
		_makefile_output_variable(fp, "DESTDIR",
				configure->prefs->destdir);
	}
	if(targets != NULL || includes != NULL || package != NULL)
	{
		_makefile_output_variable(fp, "RM", configure->programs.rm);
		_makefile_output_variable(fp, "LN", configure->programs.ln);
	}
	if(package != NULL)
	{
		_makefile_output_variable(fp, "TAR", configure->programs.tar);
		_makefile_output_variable(fp, "MKDIR",
				configure->programs.mkdir);
	}
	if(targets != NULL || includes != NULL)
	{
		if(package == NULL)
			_makefile_output_variable(fp, "MKDIR",
					configure->programs.mkdir);
		_makefile_output_variable(fp, "INSTALL",
				configure->programs.install);
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

	if((type = config_get(configure->config, target, "type")) == NULL)
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
static void _binary_ldflags(Configure * configure, FILE * fp,
		String const * ldflags);
static void _variables_binary(Configure * configure, FILE * fp, char * done)
{
	String * p;

	if(!done[TT_LIBRARY] && !done[TT_SCRIPT])
	{
		_makefile_output_variable(fp, "OBJDIR", "");
		_makefile_output_variable(fp, "PREFIX",
				configure->prefs->prefix);
		_makefile_output_variable(fp, "DESTDIR",
				configure->prefs->destdir);
	}
	/* BINDIR */
	if(configure->prefs->bindir[0] == '/')
		_makefile_output_variable(fp, "BINDIR",
				configure->prefs->bindir);
	else if((p = string_new_append("$(PREFIX)/", configure->prefs->bindir,
					NULL)) != NULL)
	{
		_makefile_output_variable(fp, "BINDIR", p);
		string_delete(p);
	}
	/* SBINDIR */
	if(configure->prefs->sbindir[0] == '/')
		_makefile_output_variable(fp, "SBINDIR",
				configure->prefs->sbindir);
	else if((p = string_new_append("$(PREFIX)/", configure->prefs->sbindir,
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
		_targets_exeext(configure, fp);
	}
}

static void _targets_asflags(Configure * configure, FILE * fp)
{
	String const * as;
	String const * asf;

	as = config_get(configure->config, NULL, "as");
	asf = config_get(configure->config, NULL, "asflags");
	if(as != NULL || asf != NULL)
	{
		_makefile_output_variable(fp, "AS", (as != NULL) ? as
				: configure->programs.as);
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

	cppf = config_get(configure->config, NULL, "cppflags_force");
	cpp = config_get(configure->config, NULL, "cppflags");
	cff = config_get(configure->config, NULL, "cflags_force");
	cf = config_get(configure->config, NULL, "cflags");
	cc = config_get(configure->config, NULL, "cc");
	if(cppf == NULL && cpp == NULL && cff == NULL && cf == NULL
			&& cc == NULL)
		return;
	if(cc == NULL)
		_makefile_output_variable(fp, "CC", configure->programs.cc);
	else
		_makefile_output_variable(fp, "CC", cc);
	_makefile_output_variable(fp, "CPPFLAGSF", cppf);
	_makefile_output_variable(fp, "CPPFLAGS", cpp);
	p = NULL;
	if(configure->os == HO_GNU_LINUX && string_find(cff, "-ansi"))
		p = string_new_append(cff, " -D _GNU_SOURCE");
	_makefile_output_variable(fp, "CFLAGSF", (p != NULL) ? p : cff);
	string_delete(p);
	p = NULL;
	if(configure->os == HO_GNU_LINUX && string_find(cf, "-ansi"))
		p = string_new_append(cf, " -D _GNU_SOURCE");
	_makefile_output_variable(fp, "CFLAGS", (p != NULL) ? p : cf);
	string_delete(p);
}

static void _targets_cxxflags(Configure * configure, FILE * fp)
{
	String const * cxx;
	String const * cxxff;
	String const * cxxf;

	cxx = config_get(configure->config, NULL, "cxx");
	cxxff = config_get(configure->config, NULL, "cxxflags_force");
	cxxf = config_get(configure->config, NULL, "cxxflags");
	if(cxx != NULL || cxxff != NULL || cxxf != NULL)
	{
		if(cxx == NULL)
			cxx = configure->programs.cxx;
		_makefile_output_variable(fp, "CXX", cxx);
	}
	if(cxxff != NULL)
	{
		fprintf(fp, "%s%s", "CXXFLAGSF= ", cxxff);
		if(configure->os == HO_GNU_LINUX && string_find(cxxff, "-ansi"))
			fprintf(fp, "%s", " -D _GNU_SOURCE");
		fputc('\n', fp);
	}
	if(cxxf != NULL)
	{
		fprintf(fp, "%s%s", "CXXFLAGS= ", cxxf);
		if(configure->os == HO_GNU_LINUX && string_find(cxxf, "-ansi"))
			fprintf(fp, "%s", " -D _GNU_SOURCE");
		fputc('\n', fp);
	}
}

static void _targets_exeext(Configure * configure, FILE * fp)
{
	if(configure->os == HO_WIN32)
		fputs("EXEEXT\t= .exe\n", fp);
}

static void _targets_ldflags(Configure * configure, FILE * fp)
{
	String const * p;

	if((p = config_get(configure->config, NULL, "ldflags_force")) != NULL)
	{
		fputs("LDFLAGSF=", fp);
		_binary_ldflags(configure, fp, p);
		fputc('\n', fp);
	}
	if((p = config_get(configure->config, NULL, "ldflags")) != NULL)
	{
		fputs("LDFLAGS\t=", fp);
		_binary_ldflags(configure, fp, p);
		fputc('\n', fp);
	}
}

static void _binary_ldflags(Configure * configure, FILE * fp,
		String const * ldflags)
{
	char const * libs_bsd[] = { "dl", "resolv", "ossaudio", "socket",
		"ws2_32", NULL };
	char const * libs_deforaos[] = { "ossaudio", "resolv", "ssl", "ws2_32",
		NULL };
	char const * libs_gnu[] = { "ossaudio", "resolv", "socket", "ws2_32",
		NULL };
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
		fprintf(fp, " %s%s", ldflags, "\n");
		return;
	}
	switch(configure->os)
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
	fprintf(fp, " %s", p);
	string_delete(p);
}

static void _variables_library(Configure * configure, FILE * fp, char * done)
{
	String const * libdir;
	String const * p;

	if(!done[TT_LIBRARY] && !done[TT_SCRIPT])
	{
		_makefile_output_variable(fp, "OBJDIR", "");
		_makefile_output_variable(fp, "PREFIX",
				configure->prefs->prefix);
		_makefile_output_variable(fp, "DESTDIR",
				configure->prefs->destdir);
	}
	if((libdir = config_get(configure->config, NULL, "libdir")) == NULL)
		libdir = configure->prefs->libdir;
	if(libdir[0] == '/')
		_makefile_output_variable(fp, "LIBDIR", libdir);
	else
		fprintf(fp, "%s%s\n", "LIBDIR\t= $(PREFIX)/", libdir);
	if(!done[TT_BINARY])
	{
		_targets_asflags(configure, fp);
		_targets_cflags(configure, fp);
		_targets_cxxflags(configure, fp);
		_targets_ldflags(configure, fp);
		_targets_exeext(configure, fp);
	}
	if(configure_can_library_static(configure))
		_variables_library_static(configure, fp);
	if((p = config_get(configure->config, NULL, "ld")) == NULL)
		_makefile_output_variable(fp, "CCSHARED",
				configure->programs.ccshared);
	else
		_makefile_output_variable(fp, "CCSHARED", p);
}

static void _variables_library_static(Configure * configure, FILE * fp)
{
	String const * p;

	if((p = config_get(configure->config, NULL, "ar")) == NULL)
		_makefile_output_variable(fp, "AR", configure->programs.ar);
	else
		_makefile_output_variable(fp, "AR", p);
	if((p = config_get(configure->config, NULL, "ranlib")) == NULL)
		_makefile_output_variable(fp, "RANLIB",
				configure->programs.ranlib);
	else
		_makefile_output_variable(fp, "RANLIB", p);
}

static void _variables_libtool(Configure * configure, FILE * fp, char * done)
{
	String const * p;

	_variables_library(configure, fp, done);
	if(!done[TT_LIBTOOL])
	{
		if((p = config_get(configure->config, NULL, "libtool")) == NULL)
			_makefile_output_variable(fp, "LIBTOOL",
					configure->programs.libtool);
		else
			_makefile_output_variable(fp, "LIBTOOL", p);
	}
}

static void _variables_script(Configure * configure, FILE * fp, char * done)
{
	if(!done[TT_BINARY] && !done[TT_LIBRARY] && !done[TT_SCRIPT])
	{
		_makefile_output_variable(fp, "OBJDIR", "");
		_makefile_output_variable(fp, "PREFIX",
				configure->prefs->prefix);
		_makefile_output_variable(fp, "DESTDIR",
				configure->prefs->destdir);
	}
}

static int _variables_includes(Configure * configure, FILE * fp)
{
	String const * includes;

	if((includes = config_get(configure->config, NULL, "includes")) == NULL)
		return 0;
	if(fp == NULL)
		return 0;
	if(configure->prefs->includedir[0] == '/')
		_makefile_output_variable(fp, "INCLUDEDIR",
				configure->prefs->includedir);
	else
		fprintf(fp, "%s%s\n", "INCLUDEDIR= $(PREFIX)/",
				configure->prefs->includedir);
	return 0;
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
	if((p = config_get(configure->config, NULL, "targets")) == NULL)
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

	if(configure->prefs->flags & PREFS_n)
		return 0;
	if(config_get(configure->config, NULL, "subdirs") != NULL)
		depends[i++] = "subdirs";
	if(config_get(configure->config, NULL, "targets") != NULL)
		depends[i++] = "$(TARGETS)";
	_makefile_target(fp, "all", depends[0], depends[1], NULL);
	return 0;
}

static int _targets_subdirs(Configure * configure, FILE * fp)
{
	String const * subdirs;

	if(configure->prefs->flags & PREFS_n)
		return 0;
	if((subdirs = config_get(configure->config, NULL, "subdirs")) != NULL)
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

	if((type = config_get(configure->config, target, "type")) == NULL)
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

static int _objs_source(Prefs * prefs, FILE * fp, String * source,
		TargetType tt);
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

	if((p = config_get(configure->config, target, "type")) != NULL)
		tt = enum_string(TT_LAST, sTargetType, p);
	if((p = config_get(configure->config, target, "sources")) == NULL)
		/* a binary target may have no sources */
		return 0;
	if((sources = string_new(p)) == NULL)
		return 1;
	q = sources;
	if(!(configure->prefs->flags & PREFS_n))
		fprintf(fp, "%s%s%s", "\n", target, "_OBJS =");
	for(i = 0; ret == 0; i++)
	{
		if(sources[i] != ',' && sources[i] != '\0')
			continue;
		c = sources[i];
		sources[i] = '\0';
		ret = _objs_source(configure->prefs, fp, sources, tt);
		if(c == '\0')
			break;
		sources += i + 1;
		i = 0;
	}
	if(!(configure->prefs->flags & PREFS_n))
		fputc('\n', fp);
	string_delete(q);
	return ret;
}

static int _objs_source(Prefs * prefs, FILE * fp, String * source,
		TargetType tt)
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
			if(prefs->flags & PREFS_n)
				break;
			fprintf(fp, " $(OBJDIR)%s%s", source,
					(tt == TT_LIBTOOL) ? ".lo" : ".o");
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
	String * q;

	if(_target_objs(configure, fp, target) != 0)
		return 1;
	if(configure->prefs->flags & PREFS_n)
		return 0;
	if(_target_flags(configure, fp, target) != 0)
		return 1;
	fputc('\n', fp);
	/* output the binary target */
	fprintf(fp, "%s%s%s%s%s%s", "$(OBJDIR)", target,
			(configure->os == HO_WIN32) ? "$(EXEEXT)" : "",
			": $(", target, "_OBJS)");
	if((p = config_get(configure->config, target, "depends")) != NULL)
	{
		if((q = string_new(p)) == NULL
				|| string_replace(&q, ",", " ") != 0)
		{
			string_delete(q);
			return error_print(PROGNAME);
		}
		fprintf(fp, " %s", q);
		string_delete(q);
	}
	fputc('\n', fp);
	/* build the binary */
	fprintf(fp, "%s%s%s%s%s%s%s%s", "\t$(CC) -o $(OBJDIR)", target,
			(configure->os == HO_WIN32) ? "$(EXEEXT)" : "", " $(",
			target, "_OBJS) $(", target, "_LDFLAGS)\n");
	return 0;
}

static void _flags_asm(Configure * configure, FILE * fp, String const * target);
static void _flags_c(Configure * configure, FILE * fp, String const * target);
static void _flags_cxx(Configure * configure, FILE * fp, String const * target);
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
	if((p = config_get(configure->config, target, "sources")) == NULL
			|| string_length(p) == 0)
	{
		if((p = config_get(configure->config, target, "type")) != NULL
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
				case OT_C_SOURCE:
					_flags_c(configure, fp, target);
					break;
				case OT_CXX_SOURCE:
					done[OT_CXX_SOURCE] = 1;
					_flags_cxx(configure, fp, target);
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

	fprintf(fp, "%s%s", target, "_ASFLAGS = $(CPPFLAGSF) $(CPPFLAGS)"
			" $(ASFLAGS)");
	if((p = config_get(configure->config, target, "asflags")) != NULL)
		fprintf(fp, " %s", p);
	fputc('\n', fp);
}

static void _flags_c(Configure * configure, FILE * fp, String const * target)
{
	String const * p;

	fprintf(fp, "%s%s", target, "_CFLAGS = $(CPPFLAGSF) $(CPPFLAGS)");
	if((p = config_get(configure->config, target, "cppflags")) != NULL)
		fprintf(fp, " %s", p);
	fprintf(fp, "%s", " $(CFLAGSF) $(CFLAGS)");
	if((p = config_get(configure->config, target, "cflags")) != NULL)
	{
		fprintf(fp, " %s", p);
		if(configure->os == HO_GNU_LINUX && string_find(p, "-ansi"))
			fprintf(fp, "%s", " -D _GNU_SOURCE");
	}
	fputc('\n', fp);
	fprintf(fp, "%s%s", target, "_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)");
	if((p = config_get(configure->config, target, "ldflags")) != NULL)
		_binary_ldflags(configure, fp, p);
	fputc('\n', fp);
}

static void _flags_cxx(Configure * configure, FILE * fp, String const * target)
{
	String const * p;

	fprintf(fp, "%s%s", target, "_CXXFLAGS = $(CPPFLAGSF) $(CPPFLAGS)"
			" $(CXXFLAGSF) $(CXXFLAGS)");
	if((p = config_get(configure->config, target, "cxxflags")) != NULL)
		fprintf(fp, " %s", p);
	fputc('\n', fp);
	fprintf(fp, "%s%s", target, "_LDFLAGS = $(LDFLAGSF) $(LDFLAGS)");
	if((p = config_get(configure->config, target, "ldflags")) != NULL)
		_binary_ldflags(configure, fp, p);
	fputc('\n', fp);
}

static int _target_library(Configure * configure, FILE * fp,
		String const * target)
{
	String const * soext;
	String const * p;
	String * q;
	String * soname;

	if(_target_objs(configure, fp, target) != 0)
		return 1;
	if(configure->prefs->flags & PREFS_n)
		return 0;
	if(_target_flags(configure, fp, target) != 0)
		return 1;
	soext = configure_get_soext(configure);
	if(configure_can_library_static(configure))
	{
		/* generate a static library */
		fprintf(fp, "\n%s%s%s%s%s", "$(OBJDIR)", target, ".a: $(", target,
				"_OBJS)");
		if((p = config_get(configure->config, target, "depends")) != NULL)
			fprintf(fp, " %s", p);
		fputc('\n', fp);
		fprintf(fp, "%s%s%s%s%s", "\t$(AR) -rc $(OBJDIR)", target,
				".a $(", target, "_OBJS)");
		if((q = malloc(strlen(target) + strlen(soext) + 3)) != NULL)
		{
			sprintf(q, "%s.a", target);
			if((p = config_get(configure->config, q, "ldflags"))
					!= NULL)
				_binary_ldflags(configure, fp, p);
		}
		fputc('\n', fp);
		fprintf(fp, "%s%s%s", "\t$(RANLIB) $(OBJDIR)", target, ".a\n");
	}
	if((p = config_get(configure->config, target, "soname")) != NULL)
		soname = string_new(p);
	else if(configure->os == HO_MACOSX)
		/* versioning is different on MacOS X */
		soname = string_new_append(target, ".0.0", soext, NULL);
	else if(configure->os == HO_WIN32)
		/* and on Windows */
		soname = string_new_append(target, soext, NULL);
	else
		soname = string_new_append(target, soext, ".0", NULL);
	if(soname == NULL)
		return 1;
	if(configure->os == HO_MACOSX)
		fprintf(fp, "\n%s%s%s%s%s%s%s%s%s%s%s%s", "$(OBJDIR)", soname,
				" $(OBJDIR)", target, ".0", soext, " $(OBJDIR)",
				target, soext, ": $(", target, "_OBJS)");
	else if(configure->os == HO_WIN32)
		fprintf(fp, "\n%s%s%s%s%s", "$(OBJDIR)", soname,
				": $(", target, "_OBJS)");
	else
		fprintf(fp, "\n%s%s%s%s%s%s%s%s%s%s", "$(OBJDIR)", soname,
				".0 $(OBJDIR)", soname, " $(OBJDIR)", target,
				soext, ": $(", target, "_OBJS)");
	if((p = config_get(configure->config, target, "depends")) != NULL)
		fprintf(fp, " %s", p);
	fputc('\n', fp);
	/* build the shared library */
	fprintf(fp, "%s%s%s", "\t$(CCSHARED) -o $(OBJDIR)", soname,
			(configure->os != HO_MACOSX
			 && configure->os != HO_WIN32) ? ".0" : "");
	if((p = config_get(configure->config, target, "install")) != NULL)
	{
		/* soname is not available on MacOS X */
		if(configure->os == HO_MACOSX)
			fprintf(fp, "%s%s%s%s%s%s", " -install_name ", p, "/",
					target, ".0", soext);
		else if(configure->os != HO_WIN32)
			fprintf(fp, "%s%s", " -Wl,-soname,", soname);
	}
	fprintf(fp, "%s%s%s%s%s", " $(", target, "_OBJS) $(", target,
			"_LDFLAGS)");
	if(q != NULL)
	{
		sprintf(q, "%s%s", target, soext);
		if((p = config_get(configure->config, q, "ldflags")) != NULL)
			_binary_ldflags(configure, fp, p);
		free(q);
	}
	fputc('\n', fp);
	if(configure->os == HO_MACOSX)
	{
		fprintf(fp, "%s%s%s%s%s%s%s", "\t$(LN) -s -- ", soname,
				" $(OBJDIR)", target, ".0", soext, "\n");
		fprintf(fp, "%s%s%s%s%s%s", "\t$(LN) -s -- ", soname,
				" $(OBJDIR)", target, soext, "\n");
	}
	else if(configure->os != HO_WIN32)
	{
		fprintf(fp, "%s%s%s%s%s", "\t$(LN) -s -- ", soname,
				".0 $(OBJDIR)", soname, "\n");
		fprintf(fp, "%s%s%s%s%s%s", "\t$(LN) -s -- ", soname,
				".0 $(OBJDIR)", target, soext, "\n");
	}
	string_delete(soname);
	return 0;
}

static int _target_libtool(Configure * configure, FILE * fp,
		String const * target)
{
	String const * p;

	if(_target_objs(configure, fp, target) != 0)
		return 1;
	if(configure->prefs->flags & PREFS_n)
		return 0;
	if(_target_flags(configure, fp, target) != 0)
		return 1;
	fputc('\n', fp);
	fprintf(fp, "%s%s%s%s%s", "$(OBJDIR)", target, ".la: $(", target,
			"_OBJS)\n");
	fprintf(fp, "%s%s%s%s%s", "\t$(LIBTOOL) --mode=link $(CC) -o $(OBJDIR)",
			target, ".la $(", target, "_OBJS)");
	if((p = config_get(configure->config, target, "ldflags")) != NULL)
		_binary_ldflags(configure, fp, p);
	fprintf(fp, "%s%s%s", " -rpath $(LIBDIR) $(", target, "_LDFLAGS)\n");
	return 0;
}

static int _target_object(Configure * configure, FILE * fp,
		String const * target)
{
	String const * p;
	String const * extension;

	if((p = config_get(configure->config, target, "sources")) == NULL)
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
	if(configure->prefs->flags & PREFS_n)
		return 0;
	if((extension = _source_extension(p)) == NULL)
		return 1;
	switch(_source_type(extension))
	{
		case OT_ASM_SOURCE:
			fprintf(fp, "\n%s%s%s%s\n%s%s", target, "_OBJS = ",
					"$(OBJDIR)", target, target, "_ASFLAGS ="
					" $(CPPFLAGSF) $(CPPFLAGS) $(ASFLAGS)");
			if((p = config_get(configure->config, target,
							"asflags")) != NULL)
				fprintf(fp, " %s", p);
			fputc('\n', fp);
			break;
		case OT_C_SOURCE:
			fprintf(fp, "\n%s%s%s%s\n%s%s", target, "_OBJS = ",
					"$(OBJDIR)", target, target, "_CFLAGS ="
					" $(CPPFLAGSF) $(CPPFLAGS) $(CFLAGSF)"
					" $(CFLAGS)");
			if((p = config_get(configure->config, target, "cflags"))
					!= NULL)
				fprintf(fp, " %s", p);
			fputc('\n', fp);
			break;
		case OT_CXX_SOURCE:
			fprintf(fp, "\n%s%s%s%s\n%s%s", target, "_OBJS = ",
					"$(OBJDIR)", target, target, "_CXXFLAGS ="
					" $(CPPFLAGSF) $(CPPFLAGS)"
					" $(CXXFLAGS)");
			if((p = config_get(configure->config, target,
							"cxxflags")) != NULL)
				fprintf(fp, " %s", p);
			fputc('\n', fp);
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
	String const * soext;
	String const * p;
	String * q;

	if(_target_objs(configure, fp, target) != 0)
		return 1;
	if(configure->prefs->flags & PREFS_n)
		return 0;
	if(_target_flags(configure, fp, target) != 0)
		return 1;
	fputc('\n', fp);
	soext = configure_get_soext(configure);
	fprintf(fp, "%s%s%s%s%s%s", "$(OBJDIR)", target, soext, ": $(",
			target, "_OBJS)");
	if((p = config_get(configure->config, target, "depends")) != NULL)
		fprintf(fp, " %s", p);
	fputc('\n', fp);
	/* build the plug-in */
	fprintf(fp, "%s%s%s%s%s%s%s%s", "\t$(CCSHARED) -o $(OBJDIR)", target,
			soext, " $(", target, "_OBJS) $(", target, "_LDFLAGS)");
	if((q = malloc(strlen(target) + strlen(soext) + 1)) != NULL)
	{
		sprintf(q, "%s%s", target, soext);
		if((p = config_get(configure->config, q, "ldflags")) != NULL)
			_binary_ldflags(configure, fp, p);
		free(q);
	}
	fputc('\n', fp);
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

	if((p = config_get(configure->config, NULL, "targets")) == NULL)
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

static int _script_depends(Config * config, FILE * fp, String const * target);
static int _target_script(Configure * configure, FILE * fp,
		String const * target)
{
	String const * script;

	if((script = config_get(configure->config, target, "script")) == NULL)
	{
		fprintf(stderr, "%s%s%s", PROGNAME ": ", target,
				": No script for target\n");
		return 1;
	}
	if(configure->prefs->flags & PREFS_S)
		error_set_print(PROGNAME, 0, "%s: %s%s%s", target, "The \"",
				script,
				"\" script is executed while compiling");
	if(configure->prefs->flags & PREFS_n)
		return 0;
	fprintf(fp, "\n$(OBJDIR)%s:", target);
	_script_depends(configure->config, fp, target);
	fputc('\n', fp);
	fprintf(fp, "\t%s -P \"$(PREFIX)\" -- \"$(OBJDIR)%s\"\n", script,
			target);
	return 0;
}

static int _script_depends(Config * config, FILE * fp, String const * target)
{
	String const * p;
	String * depends;
	String * q;
	size_t i;
	char c;

	/* XXX code duplication */
	if((p = config_get(config, target, "depends")) == NULL)
		return 0;
	if((depends = string_new(p)) == NULL)
		return 1;
	q = depends;
	for(i = 0;; i++)
	{
		if(depends[i] != ',' && depends[i] != '\0')
			continue;
		c = depends[i];
		depends[i] = '\0';
		fprintf(fp, " %s", depends);
		if(c == '\0')
			break;
		depends += i + 1;
		i = 0;
	}
	string_delete(q);
	return 0;
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

	if((p = config_get(configure->config, target, "sources")) == NULL)
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

static int _source_depends(Config * config, FILE * fp, String const * source);
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

	if((p = config_get(configure->config, target, "type")) != NULL)
			tt = enum_string(TT_LAST, sTargetType, p);
	if((extension = _source_extension(source)) == NULL)
		return 1;
	len = string_length(source) - string_length(extension) - 1;
	source[len] = '\0';
	switch((ot = _source_type(extension)))
	{
		case OT_ASM_SOURCE:
			if(configure->prefs->flags & PREFS_n)
				break;
			if(tt == TT_OBJECT)
				fprintf(fp, "\n$(OBJDIR)%s", target);
			else
				fprintf(fp, "\n$(OBJDIR)%s%s", source, ".o");
			if(tt == TT_LIBTOOL)
				fprintf(fp, " %s.lo", source);
			fprintf(fp, "%s%s%s%s", ": ", source, ".", extension);
			source[len] = '.'; /* FIXME ugly */
			_source_depends(configure->config, fp, source);
			source[len] = '\0';
			fputs("\n\t", fp);
			if(tt == TT_LIBTOOL)
				fputs("$(LIBTOOL) --mode=compile ", fp);
			fprintf(fp, "%s%s%s", "$(AS) $(", target, "_ASFLAGS)");
			if(tt == TT_OBJECT)
				fprintf(fp, "%s%s%s%s%s%s", " -o $(OBJDIR)",
						target, " ", source, ".",
						extension);
			else
				fprintf(fp, "%s%s%s%s%s%s", " -o $(OBJDIR)",
						source, ".o ", source, ".",
						extension);
			fputc('\n', fp);
			break;
		case OT_C_SOURCE:
			if(configure->prefs->flags & PREFS_n)
				break;
			if(tt == TT_OBJECT)
				fprintf(fp, "\n$(OBJDIR)%s", target);
			else
				fprintf(fp, "\n$(OBJDIR)%s%s", source, ".o");
			if(tt == TT_LIBTOOL)
				fprintf(fp, " %s%s", source, ".lo");
			fprintf(fp, "%s%s%s%s", ": ", source, ".", extension);
			source[len] = '.'; /* FIXME ugly */
			_source_depends(configure->config, fp, source);
			/* FIXME do both wherever also relevant */
			p = config_get(configure->config, source, "cppflags");
			q = config_get(configure->config, source, "cflags");
			source[len] = '\0';
			fputs("\n\t", fp);
			if(tt == TT_LIBTOOL)
				fputs("$(LIBTOOL) --mode=compile ", fp);
			fputs("$(CC)", fp);
			if(p != NULL)
				fprintf(fp, " %s", p);
			fprintf(fp, "%s%s%s", " $(", target, "_CFLAGS)");
			if(q != NULL)
			{
				fprintf(fp, " %s", q);
				if(configure->os == HO_GNU_LINUX
					       	&& string_find(q, "-ansi"))
					fputs(" -D _GNU_SOURCE", fp);
			}
			if(tt == TT_OBJECT)
				fprintf(fp, "%s%s", " -o $(OBJDIR)", target);
			else
				fprintf(fp, "%s%s%s", " -o $(OBJDIR)", source,
						".o");
			fprintf(fp, "%s%s%s%s", " -c ", source, ".", extension);
			fputc('\n', fp);
			break;
		case OT_CXX_SOURCE:
			if(configure->prefs->flags & PREFS_n)
				break;
			if(tt == TT_OBJECT)
				fprintf(fp, "\n$(OBJDIR)%s", target);
			else
				fprintf(fp, "\n$(OBJDIR)%s%s", source, ".o");
			fprintf(fp, "%s%s%s%s", ": ", source, ".", extension);
			source[len] = '.'; /* FIXME ugly */
			_source_depends(configure->config, fp, source);
			p = config_get(configure->config, source, "cxxflags");
			source[len] = '\0';
			fprintf(fp, "%s%s%s", "\n\t$(CXX) $(", target,
					"_CXXFLAGS)");
			if(p != NULL)
				fprintf(fp, " %s", p);
			if(tt == TT_OBJECT)
				fprintf(fp, "%s%s", " -o $(OBJDIR)", target);
			else
				fprintf(fp, "%s%s%s", " -o $(OBJDIR)", source,
						".o");
			fprintf(fp, "%s%s%s%s", " -c ", source, ".", extension);
			fputc('\n', fp);
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

static int _source_depends(Config * config, FILE * fp, String const * target)
{
	String const * p;
	String * depends;
	String * q;
	size_t i;
	char c;

	if((p = config_get(config, target, "depends")) == NULL)
		return 0;
	if((depends = string_new(p)) == NULL)
		return 1;
	q = depends;
	for(i = 0;; i++)
	{
		if(depends[i] != ',' && depends[i] != '\0')
			continue;
		c = depends[i];
		depends[i] = '\0';
		fprintf(fp, " %s", depends);
		if(c == '\0')
			break;
		depends += i + 1;
		i = 0;
	}
	string_delete(q);
	return 0;
}

static int _clean_targets(Config * config, FILE * fp);
static int _write_clean(Configure * configure, FILE * fp)
{
	if(configure->prefs->flags & PREFS_n)
		return 0;
	_makefile_target(fp, "clean", NULL);
	if(config_get(configure->config, NULL, "subdirs") != NULL)
		_makefile_subdirs(fp, "clean");
	return _clean_targets(configure->config, fp);
}

static int _clean_targets(Config * config, FILE * fp)
{
	String const * p;
	String * targets;
	String * q;
	size_t i;
	char c;

	if((p = config_get(config, NULL, "targets")) == NULL)
		return 0;
	if((targets = string_new(p)) == NULL)
		return 1;
	q = targets;
	/* remove all of the object files */
	fputs("\t$(RM) --", fp);
	for(i = 0;; i++)
	{
		if(targets[i] != ',' && targets[i] != '\0')
			continue;
		c = targets[i];
		targets[i] = '\0';
		fprintf(fp, "%s%s%s", " $(", targets, "_OBJS)");
		if(c == '\0')
			break;
		targets[i] = c;
		targets += i + 1;
		i = 0;
	}
	fputc('\n', fp);
	targets = q;
	/* let each scripted target remove the relevant object files */
	for(i = 0;; i++)
	{
		if(targets[i] != ',' && targets[i] != '\0')
			continue;
		c = targets[i];
		targets[i] = '\0';
		if((p = config_get(config, targets, "type")) != NULL
				&& strcmp(p, "script") == 0
				&& (p = config_get(config, targets, "script"))
				!= NULL)
			fprintf(fp, "\t%s%s%s%s\n", p, " -c -P \"$(PREFIX)\""
					" -- \"", targets, "\"");
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

	if(configure->prefs->flags & PREFS_n)
		return 0;
	/* only depend on the "clean" target if we do not have subfolders */
	if((subdirs = config_get(configure->config, NULL, "subdirs")) == NULL)
		_makefile_target(fp, "distclean", "clean", NULL);
	else
	{
		_makefile_target(fp, "distclean", NULL);
		_makefile_subdirs(fp, "distclean");
		_clean_targets(configure->config, fp);
	}
	/* FIXME do not erase targets that need be distributed */
	if(config_get(configure->config, NULL, "targets") != NULL)
		_makefile_remove(fp, 0, "$(TARGETS)", NULL);
	return 0;
}

static int _dist_subdir(Config * config, FILE * fp, Config * subdir);
static int _write_dist(Configure * configure, FILE * fp, configArray * ca,
	       	int from, int to)
{
	String const * package;
	String const * version;
	Config * p;
	int i;

	if(configure->prefs->flags & PREFS_n)
		return 0;
	package = config_get(configure->config, NULL, "package");
	version = config_get(configure->config, NULL, "version");
	if(package == NULL || version == NULL)
		return 0;
	_makefile_target(fp, "dist", NULL);
	_makefile_remove(fp, 1, "$(OBJDIR)$(PACKAGE)-$(VERSION)", NULL);
	_makefile_link(fp, 1, "\"$$PWD\"", "$(OBJDIR)$(PACKAGE)-$(VERSION)");
	fputs("\t@cd $(OBJDIR). && $(TAR) -czvf $(OBJDIR)$(PACKAGE)-$(VERSION).tar.gz -- \\\n",
			fp);
	for(i = from + 1; i < to; i++)
	{
		array_get_copy(ca, i, &p);
		_dist_subdir(configure->config, fp, p);
	}
	if(from < to)
	{
		array_get_copy(ca, from, &p);
		_dist_subdir(configure->config, fp, p);
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
	const char target[] = "\t(cd \"$(PACKAGE)-$(VERSION)\" && $(MAKE) OBJDIR=\"$$PWD/objdir/\")\n"
		"\t(cd \"$(PACKAGE)-$(VERSION)\" && $(MAKE) OBJDIR=\"$$PWD/objdir/\" DESTDIR=\"$$PWD/destdir\" install)\n"
		"\t(cd \"$(PACKAGE)-$(VERSION)\" && $(MAKE) OBJDIR=\"$$PWD/objdir/\" DESTDIR=\"$$PWD/destdir\" uninstall)\n"
		"\t(cd \"$(PACKAGE)-$(VERSION)\" && $(MAKE) OBJDIR=\"$$PWD/objdir/\" distclean)\n"
		"\t(cd \"$(PACKAGE)-$(VERSION)\" && $(MAKE) dist)\n";
	const char posttarget[] = "\t$(RM) -r -- $(PACKAGE)-$(VERSION)\n";

	if(configure->prefs->flags & PREFS_n)
		return 0;
	package = config_get(configure->config, NULL, "package");
	version = config_get(configure->config, NULL, "version");
	if(package == NULL || version == NULL)
		return 0;
	fputs(pretarget, fp);
	fputs(target, fp);
	fputs(posttarget, fp);
	return 0;
}

static int _dist_subdir_dist(FILE * fp, String const * path,
		String const * dist);
static int _dist_subdir(Config * config, FILE * fp, Config * subdir)
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

	path = config_get(config, NULL, "directory");
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
	fprintf(fp, "%s%s%s%s%s%s%s%s", "\t\t", quote, "$(PACKAGE)-$(VERSION)/",
			path, (path[0] == '\0') ? "" : "/", PROJECT_CONF, quote,
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
		fprintf(fp, "%s%s%s%s%s%s%s%s", "\t\t", quote,
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

	if(configure->prefs->flags & PREFS_n)
		return 0;
	targets = config_get(configure->config, NULL, "targets");
	_makefile_target(fp, "install", (targets != NULL) ? "$(TARGETS)" : NULL,
			NULL);
	if(config_get(configure->config, NULL, "subdirs") != NULL)
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

	if((p = config_get(configure->config, NULL, "targets")) == NULL)
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

	if((type = config_get(configure->config, target, "type")) == NULL)
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
	String const * exeext;

	if((path = config_get(configure->config, target, "install")) == NULL)
		return;
	exeext = (configure->os == HO_WIN32) ? "$(EXEEXT)" : "";
	fprintf(fp, "%s%s\n", "\t$(MKDIR) $(DESTDIR)", path);
	fprintf(fp, "%s%s%s%s%s/%s%s\n", "\t$(INSTALL) -m 0755 $(OBJDIR)",
			target, exeext, " $(DESTDIR)", path, target, exeext);
}

static int _install_target_library(Configure * configure, FILE * fp,
		String const * target)
{
	String const * path;
	String const * soext;
	String const * p;
	String * soname;

	if((path = config_get(configure->config, target, "install")) == NULL)
		return 0;
	soext = configure_get_soext(configure);
	fprintf(fp, "%s%s\n", "\t$(MKDIR) $(DESTDIR)", path);
	if(configure_can_library_static(configure))
		/* install the static library */
		fprintf(fp, "%s%s%s%s/%s%s", "\t$(INSTALL) -m 0644 $(OBJDIR)",
				target, ".a $(DESTDIR)", path, target, ".a\n");
	if((p = config_get(configure->config, target, "soname")) != NULL)
		soname = string_new(p);
	else if(configure->os == HO_MACOSX)
		/* versioning is different on MacOS X */
		soname = string_new_append(target, ".0.0", soext, NULL);
	else if(configure->os == HO_WIN32)
		/* and on Windows */
		soname = string_new_append(target, soext, NULL);
	else
		soname = string_new_append(target, soext, ".0", NULL);
	if(soname == NULL)
		return 1;
	/* install the shared library */
	if(configure->os == HO_MACOSX)
	{
		fprintf(fp, "%s%s%s%s/%s%s", "\t$(INSTALL) -m 0755 $(OBJDIR)",
				soname, " $(DESTDIR)", path, soname, "\n");
		fprintf(fp, "%s%s%s%s/%s%s%s%s", "\t$(LN) -s -- ", soname,
				" $(DESTDIR)", path, target, ".0", soext, "\n");
		fprintf(fp, "%s%s%s%s/%s%s%s", "\t$(LN) -s -- ", soname,
				" $(DESTDIR)", path, target, soext, "\n");
	}
	else if(configure->os == HO_WIN32)
		fprintf(fp, "%s%s%s%s%s%s%s", "\t$(INSTALL) -m 0755 $(OBJDIR)",
				soname, " $(DESTDIR)", path, "/", soname, "\n");
	else
	{
		fprintf(fp, "%s%s%s%s/%s%s", "\t$(INSTALL) -m 0755 $(OBJDIR)",
				soname, ".0 $(DESTDIR)", path, soname, ".0\n");
		fprintf(fp, "%s%s%s%s/%s%s", "\t$(LN) -s -- ", soname,
				".0 $(DESTDIR)", path, soname, "\n");
		fprintf(fp, "%s%s%s%s/%s%s%s", "\t$(LN) -s -- ", soname,
				".0 $(DESTDIR)", path, target, soext, "\n");
	}
	string_delete(soname);
	return 0;
}

static void _install_target_libtool(Configure * configure, FILE * fp,
		String const * target)
{
	String const * path;

	if((path = config_get(configure->config, target, "install")) == NULL)
		return;
	fprintf(fp, "%s%s\n", "\t$(MKDIR) $(DESTDIR)", path);
	fprintf(fp, "%s%s%s%s/%s%s", "\t$(LIBTOOL) --mode=install $(INSTALL)"
			" -m 0755 $(OBJDIR)", target, ".la $(DESTDIR)", path,
			target, ".la\n");
	fprintf(fp, "%s/%s\n", "\t$(LIBTOOL) --mode=finish $(DESTDIR)", path);
}

static void _install_target_object(Configure * configure, FILE * fp,
		String const * target)
{
	String const * path;

	if((path = config_get(configure->config, target, "install")) == NULL)
		return;
	fprintf(fp, "%s%s\n", "\t$(MKDIR) $(DESTDIR)", path);
	fprintf(fp, "%s%s%s%s/%s\n", "\t$(INSTALL) -m 0644 $(OBJDIR)", target,
			" $(DESTDIR)", path, target);
}

static void _install_target_plugin(Configure * configure, FILE * fp,
		String const * target)
{
	String const * path;
	String const * soext;
	String const * mode;
	mode_t m = 0755;
	String * p;

	if((path = config_get(configure->config, target, "install")) == NULL)
		return;
	soext = configure_get_soext(configure);
	if((mode = config_get(configure->config, target, "mode")) == NULL
			/* XXX these tests are not sufficient */
			|| mode[0] == '\0'
			|| (m = strtol(mode, &p, 8)) == 0
			|| *p != '\0')
		mode = "0755";
	fprintf(fp, "%s%s\n", "\t$(MKDIR) $(DESTDIR)", path);
	fprintf(fp, "%s%04o%s%s%s%s%s%s%s%s%s", "\t$(INSTALL) -m ", m,
			" $(OBJDIR)", target, soext, " $(DESTDIR)", path, "/",
			target, soext, "\n");
}

static void _install_target_script(Configure * configure, FILE * fp,
		String const * target)
{
	String const * path;
	String const * script;

	if((path = config_get(configure->config, target, "install")) == NULL)
		return;
	if((script = config_get(configure->config, target, "script")) == NULL)
		return;
	fprintf(fp, "\t%s%s%s%s%s%s", script, " -P \"$(DESTDIR)",
			(path[0] != '\0') ? path : "$(PREFIX)",
			"\" -i -- \"$(OBJDIR)", target, "\"\n");
}

static int _install_include(Config * config, FILE * fp, String const * include);
static int _install_includes(Configure * configure, FILE * fp)
{
	int ret = 0;
	String const * p;
	String * includes;
	String * q;
	size_t i;
	char c;

	if((p = config_get(configure->config, NULL, "includes")) == NULL)
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
		ret |= _install_include(configure->config, fp,
				includes);
		if(c == '\0')
			break;
		includes += i + 1;
		i = 0;
	}
	string_delete(q);
	return ret;
}

static int _install_include(Config * config, FILE * fp, String const * include)
{
	char const * install;

	if((install = config_get(config, include, "install")) == NULL)
		install = "$(INCLUDEDIR)";
	fprintf(fp, "%s%s\n", "\t$(MKDIR) $(DESTDIR)", install);
	fprintf(fp, "%s%s%s%s/%s\n", "\t$(INSTALL) -m 0644 ", include,
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

	if((p = config_get(configure->config, NULL, "dist")) == NULL)
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
		if((mode = config_get(configure->config, dist, "mode")) == NULL)
			mode = "0644";
		ret |= _dist_check(configure, dist, mode);
		if((d = config_get(configure->config, dist, "install")) != NULL)
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
	if((configure->prefs->flags & PREFS_S) && (m & 04000))
		error_set_print(PROGNAME, 0, "%s: %s", target,
				"Installed as a SUID file");
	if((configure->prefs->flags & PREFS_S) && (m & 04000))
		error_set_print(PROGNAME, 0, "%s: %s", target,
				"Installed as a SGID file");
	if((configure->prefs->flags & PREFS_S) && (m & 0111))
		error_set_print(PROGNAME, 0, "%s: %s", target,
				"Installed as an executable file");
	if((configure->prefs->flags & PREFS_S) && (m & 0020))
		error_set_print(PROGNAME, 0, "%s: %s", target,
				"Installed as a group-writable file");
	if((configure->prefs->flags & PREFS_S) && (m & 0002))
		error_set_print(PROGNAME, 0, "%s: %s", target,
				"Installed as a writable file");
	return 0;
}

static int _dist_install(Configure * configure, FILE * fp,
		char const * directory, char const * mode,
		char const * filename)
{
	char sep = (configure->os != HO_WIN32) ? '/' : '\\';
	String * p;
	char const * q;

	if(strchr(filename, sep) != NULL)
	{
		if((p = string_new(filename)) == NULL)
			return -1;
		q = dirname(p);
		fprintf(fp, "%s%s%c%s\n", "\t$(MKDIR) $(DESTDIR)", directory,
				sep, q);
		string_delete(p);
	}
	else
		fprintf(fp, "%s%s\n", "\t$(MKDIR) $(DESTDIR)", directory);
	fprintf(fp, "%s%s%s%s%s%s%c%s\n", "\t$(INSTALL) -m ", mode, " ",
			filename, " $(DESTDIR)", directory, sep, filename);
	return 0;
}

static int _uninstall_target(Configure * configure, FILE * fp,
		String const * target);
static int _uninstall_include(Config * config, FILE * fp,
		String const * include);
static int _uninstall_dist(Config * config, FILE * fp, String const * dist);
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

	if(configure->prefs->flags & PREFS_n)
		return 0;
	_makefile_target(fp, "uninstall", NULL);
	if(config_get(configure->config, NULL, "subdirs") != NULL)
		_makefile_subdirs(fp, "uninstall");
	if((p = config_get(configure->config, NULL, "targets")) != NULL)
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
	if((p = config_get(configure->config, NULL, "includes")) != NULL)
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
			ret = _uninstall_include(configure->config, fp,
					includes);
			if(c == '\0')
				break;
			includes += i + 1;
			i = 0;
		}
		string_delete(q);
	}
	if((p = config_get(configure->config, NULL, "dist")) != NULL)
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
			ret = _uninstall_dist(configure->config, fp, dist);
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
	String const * exeext;
	String const * soext;
	TargetType tt;
	const String rm_destdir[] = "$(RM) -- $(DESTDIR)";

	if((type = config_get(configure->config, target, "type")) == NULL)
		return 1;
	if((path = config_get(configure->config, target, "install")) == NULL)
		return 0;
	exeext = (configure->os == HO_WIN32) ? "$(EXEEXT)" : "";
	soext = configure_get_soext(configure);
	tt = enum_string(TT_LAST, sTargetType, type);
	switch(tt)
	{
		case TT_BINARY:
			fprintf(fp, "\t%s%s/%s%s\n", rm_destdir, path, target,
					exeext);
			break;
		case TT_LIBRARY:
			if(_uninstall_target_library(configure, fp, target,
						path) != 0)
				return 1;
			break;
		case TT_LIBTOOL:
			fprintf(fp, "\t%s%s%s/%s%s", "$(LIBTOOL)"
					" --mode=uninstall ", rm_destdir, path,
					target, ".la\n");
			break;
		case TT_OBJECT:
			fprintf(fp, "\t%s%s/%s\n", rm_destdir, path, target);
			break;
		case TT_PLUGIN:
			fprintf(fp, "\t%s%s/%s%s\n", rm_destdir, path, target,
					soext);
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
	String const * soext;
	String * soname;
	String const * p;
	const String format[] = "\t%s%s/%s%s%s%s";
	const String rm_destdir[] = "$(RM) -- $(DESTDIR)";

	soext = configure_get_soext(configure);
	if(configure_can_library_static(configure))
		/* uninstall the static library */
		fprintf(fp, format, rm_destdir, path, target, ".a\n", "", "");
	if((p = config_get(configure->config, target, "soname")) != NULL)
		soname = string_new(p);
	else if(configure->os == HO_MACOSX)
		/* versioning is different on MacOS X */
		soname = string_new_append(target, ".0.0", soext, NULL);
	else if(configure->os == HO_WIN32)
		/* and on Windows */
		soname = string_new_append(target, soext, NULL);
	else
		soname = string_new_append(target, soext, ".0", NULL);
	if(soname == NULL)
		return 1;
	/* uninstall the shared library */
	if(configure->os == HO_MACOSX)
	{
		fprintf(fp, format, rm_destdir, path, soname, "\n", "", "");
		fprintf(fp, format, rm_destdir, path, target, ".0", soext,
				"\n");
	}
	else if(configure->os != HO_WIN32)
	{
		fprintf(fp, format, rm_destdir, path, soname, ".0\n", "", "");
		fprintf(fp, format, rm_destdir, path, soname, "\n", "", "");
	}
	fprintf(fp, format, rm_destdir, path, target, soext, "\n", "");
	string_delete(soname);
	return 0;
}

static void _uninstall_target_script(Configure * configure, FILE * fp,
		String const * target, String const * path)
{
	String const * script;

	if((script = config_get(configure->config, target, "script")) == NULL)
		return;
	fprintf(fp, "\t%s%s%s%s%s%s", script, " -P \"$(DESTDIR)",
			(path[0] != '\0') ? path : "$(PREFIX)", "\" -u -- \"",
			target, "\"\n");
}

static int _uninstall_include(Config * config, FILE * fp,
		String const * include)
{
	String const * install;

	if((install = config_get(config, include, "install")) == NULL)
		install = "$(INCLUDEDIR)";
	fprintf(fp, "%s%s/%s\n", "\t$(RM) -- $(DESTDIR)", install, include);
	return 0;
}

static int _uninstall_dist(Config * config, FILE * fp, String const * dist)
{
	String const * install;

	if((install = config_get(config, dist, "install")) == NULL)
		return 0;
	fprintf(fp, "%s%s/%s\n", "\t$(RM) -- $(DESTDIR)", install, dist);
	return 0;
}


/* makefile_link */
static int _makefile_link(FILE * fp, int symlink, char const * link,
		char const * path)
{
	fprintf(fp, "\t$(LN)%s -- %s %s\n", symlink ? " -s" : "", link, path);
	return 0;
}


/* makefile_output_variable */
static int _makefile_output_variable(FILE * fp, char const * name,
		char const * value)
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
	res = fprintf(fp, "%s%s%s%s\n", name, align, equals, value);
	return (res >= 0) ? 0 : -1;
}


/* makefile_remove */
static int _makefile_remove(FILE * fp, int recursive, ...)
{
	va_list ap;
	char const * sep = " -- ";
	char const * p;

	fprintf(fp, "\t$(RM)%s", recursive ? " -r" : "");
	va_start(ap, recursive);
	while((p = va_arg(ap, char const * )) != NULL)
	{
		fprintf(fp, "%s%s", sep, p);
		sep = " ";
	}
	va_end(ap);
	fputc('\n', fp);
	return 0;
}


/* makefile_subdirs */
static int _makefile_subdirs(FILE * fp, char const * target)
{
	fprintf(fp, "\t@for i in $(SUBDIRS); do (cd \"$$i\" && $(MAKE)%s%s)"
			" || exit; done\n",
			(target != NULL) ? " " : "",
			(target != NULL) ? target : "");
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
	fprintf(fp, "\n%s:", target);
	va_start(ap, target);
	while((p = va_arg(ap, char const *)) != NULL)
		fprintf(fp, "%s%s", sep, p);
	va_end(ap);
	fputc('\n', fp);
	return 0;
}


/* makefile_targetv */
static int _makefile_targetv(FILE * fp, char const * target,
		char const ** depends)
{
	char const ** p;

	if(target == NULL)
		return -1;
	fprintf(fp, "\n%s:", target);
	if(depends != NULL)
		for(p = depends; *p != NULL; p++)
			fprintf(fp, " %s", *p);
	fputc('\n', fp);
	return 0;
}
