/* $Id$ */
/* Copyright (c) 2004-2017 Pierre Pronchery <khorben@defora.org> */
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
#include <sys/types.h>
#ifndef __WIN32__
# include <sys/utsname.h>
#endif
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "makefile.h"
#include "configure.h"
#include "../config.h"

#ifndef PROGNAME
# define PROGNAME PACKAGE
#endif

#ifndef PREFIX
# define PREFIX		"/usr/local"
#endif
#ifndef DATADIR
# define DATADIR	PREFIX "/share"
#endif


/* Configure */
/* private */
/* types */
struct _Configure {
	ConfigurePrefs * prefs;
	Config * project;
	HostArch arch;
	HostOS os;
	HostKernel kernel;
	Config * programs;
};


/* public */
/* constants */
const String * sHostArch[HA_COUNT] =
{
	"amd64", "i386", "i486", "i586", "i686", "sparc", "sparc64", "unknown"
};
const EnumMap mHostArch[] =
{
	{ HA_AMD64,	"amd64"		},
	{ HA_AMD64,	"x86_64"	},
	{ HA_ARM,	"zaurus"	},
	{ HA_I386,	"i386"		},
	{ HA_I486,	"i486"		},
	{ HA_I586,	"i586"		},
	{ HA_I686,	"i686"		},
	{ HA_SPARC,	"sparc"		},
	{ HA_SPARC,	"sun4u"		},
	{ HA_SPARC64,	"sparc64"	},
	{ HA_UNKNOWN,	"unknown"	}
};
const String * sHostOS[HO_COUNT] =
{
	"DeforaOS",
	"Darwin",
	"Linux",
	"FreeBSD", "NetBSD", "OpenBSD",
	"SunOS",
	"Windows",
	"unknown"
};
const HostKernelMap mHostKernel[] =
{
	{ HO_GNU_LINUX,	"2.0",		NULL,		NULL	},
	{ HO_GNU_LINUX,	"2.2",		NULL,		NULL	},
	{ HO_GNU_LINUX,	"2.4",		NULL,		NULL	},
	{ HO_GNU_LINUX,	"2.6",		NULL,		NULL	},
	{ HO_MACOSX,	"10.5.0",	"MacOS X",	"10.6.5"},
	{ HO_MACOSX,	"11.2.0",	"MacOS X",	"10.7.2"},
	{ HO_NETBSD,	"2.0",		NULL,		NULL	},
	{ HO_NETBSD,	"3.0",		NULL,		NULL	},
	{ HO_NETBSD,	"4.0",		NULL,		NULL	},
	{ HO_NETBSD,	"5.0",		NULL,		NULL	},
	{ HO_NETBSD,	"5.1",		NULL,		NULL	},
	{ HO_OPENBSD,	"4.0",		NULL,		NULL	},
	{ HO_OPENBSD,	"4.1",		NULL,		NULL	},
	{ HO_SUNOS,	"5.7",		NULL,		NULL	},
	{ HO_SUNOS,	"5.8",		NULL,		NULL	},
	{ HO_SUNOS,	"5.9",		NULL,		NULL	},
	{ HO_SUNOS,	"5.10",		NULL,		NULL	},
	{ HO_UNKNOWN,	"unknown",	NULL,		NULL	}
};

const String * sTargetType[TT_COUNT] = { "binary", "library", "libtool",
	"object", "plugin", "script", NULL };
const struct ExtensionType _sExtensionType[] =
{
	{ "c",		OT_C_SOURCE		},
	{ "cc",		OT_CXX_SOURCE		},
	{ "cpp",	OT_CXX_SOURCE		},
	{ "cxx",	OT_CXX_SOURCE		},
	{ "c++",	OT_CXX_SOURCE		},
	{ "asm",	OT_ASM_SOURCE		},
	{ "S",		OT_ASM_SOURCE		},
	{ "m",		OT_OBJC_SOURCE		},
	{ "mm",		OT_OBJCXX_SOURCE	},
	{ "v",		OT_VERILOG_SOURCE	},
	{ NULL,		OT_UNKNOWN		}
};
const struct ExtensionType * sExtensionType = _sExtensionType;


/* prototypes */
String const * _source_extension(String const * source);
ObjectType _source_type(String const * source);


/* functions */
/* configure_error */
int configure_error(int ret, char const * format, ...)
{
	va_list ap;

	fputs(PROGNAME ": ", stderr);
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	fputc('\n', stderr);
	va_end(ap);
	return ret;
}


/* configure_warning */
int configure_warning(int ret, char const * format, ...)
{
	va_list ap;

	fputs(PROGNAME ": warning: ", stderr);
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	fputc('\n', stderr);
	va_end(ap);
	return ret;
}


/* enum_map_find */
unsigned int enum_map_find(unsigned int last, EnumMap const * map,
		String const * str)
{
	unsigned int i;

	for(i = 0; map[i].value != last; i++)
		if(string_compare(map[i].string, str) == 0)
			return map[i].value;
	return last;
}


/* enum_string */
unsigned int enum_string(unsigned int last, const String * strings[],
		String const * str)
{
	unsigned int i;

	for(i = 0; i < last; i++)
		if(string_compare(strings[i], str) == 0)
			return i;
	return last;
}


/* enum_string_short */
unsigned int enum_string_short(unsigned int last, const String * strings[],
		String const * str)
{
	unsigned int i;

	for(i = 0; i < last; i++)
		if(string_compare_length(strings[i], str,
					string_length(strings[i])) == 0)
			return i;
	return last;
}


/* configure */
static void _configure_detect(Configure * configure);
static HostKernel _detect_kernel(HostOS os, char const * release);
static int _configure_detect_programs(Configure * configure);
static int _configure_load(ConfigurePrefs * prefs, char const * directory,
		configArray * ca);
static int _load_subdirs(ConfigurePrefs * prefs, char const * directory,
		configArray * ca, String const * subdirs);
static int _load_subdirs_subdir(ConfigurePrefs * prefs, char const * directory,
		configArray * ca, char const * subdir);
static int _configure_do(Configure * configure, configArray * ca);

int configure(ConfigurePrefs * prefs, String const * directory)
{
	int ret;
	Configure cfgr;
	configArray * ca;
	const int flags = prefs->flags;
	int i;
	Config * p;

	if((ca = configarray_new()) == NULL)
		return error_print(PROGNAME);
	cfgr.prefs = prefs;
	_configure_detect(&cfgr);
	if((ret = _configure_detect_programs(&cfgr)) != 0
			|| (ret = _configure_load(prefs, directory, ca)) == 0)
	{
		if(prefs->flags & PREFS_n)
			ret = _configure_do(&cfgr, ca);
		else
		{
			prefs->flags = PREFS_n;
			if((ret = _configure_do(&cfgr, ca)) == 0)
			{
				prefs->flags = flags;
				ret = _configure_do(&cfgr, ca);
			}
		}
	}
	for(i = array_count(ca); i > 0; i--)
	{
		array_get_copy(ca, i - 1, &p);
		config_delete(p);
	}
	array_delete(ca);
	if(cfgr.programs != NULL)
		config_delete(cfgr.programs);
	return ret;
}

static void _configure_detect(Configure * configure)
{
	char const * os;
	char const * version;
#ifdef __WIN32__

	configure->arch = HA_I386;
	configure->os = HO_WIN32;
	configure->kernel = HK_UNKNOWN;
#else
	struct utsname un;

	if(uname(&un) < 0)
	{
		configure_error(0, "%s", "system detection failed");
		configure->arch = HA_UNKNOWN;
		configure->os = HO_UNKNOWN;
		configure->kernel = HK_UNKNOWN;
		return;
	}
	configure->arch = enum_map_find(HA_LAST, mHostArch, un.machine);
	configure->os = enum_string(HO_LAST, sHostOS,
			(configure->prefs->os != NULL)
			? configure->prefs->os : un.sysname);
	configure->kernel = _detect_kernel(configure->os, un.release);
#endif
	if(configure->prefs->flags & PREFS_v)
	{
		os = (mHostKernel[configure->kernel].os_display != NULL)
			? mHostKernel[configure->kernel].os_display
			: sHostOS[configure->os];
		version = (mHostKernel[configure->kernel].version_display
				!= NULL)
			? mHostKernel[configure->kernel].version_display
			: mHostKernel[configure->kernel].version;
		printf("Detected system %s version %s (%s)\n", os, version,
				sHostArch[configure->arch]);
	}
}

static HostKernel _detect_kernel(HostOS os, char const * release)
{
	unsigned int i;

	for(i = 0; i != HK_LAST; i++)
	{
		if(mHostKernel[i].os < os)
			continue;
		if(mHostKernel[i].os > os)
			return HK_UNKNOWN;
		if(strncmp(release, mHostKernel[i].version,
					strlen(mHostKernel[i].version)) == 0)
			return i;
	}
	return i;
}

static int _configure_detect_programs(Configure * configure)
{
	int ret = 0;
	String const section[] = "programs";
	String * filename = DATADIR "/" PACKAGE "/" PACKAGE ".conf";
	struct
	{
		String const * name;
		String const * program;
	} programs[] =
	{
		{ "ccshared",	"$(CC) -shared"		},
		{ "cxx",	"c++"			},
		{ "ln",		"ln -f"			},
		{ "mkdir",	"mkdir -m 0755 -p"	},
		{ "rm",		"rm -f"			},
	};
	size_t i;

	if((configure->programs = config_new()) == NULL)
		return -1;
	for(i = 0; i < sizeof(programs) / sizeof(*programs); i++)
		if(config_set(configure->programs, section, programs[i].name,
					programs[i].program) != 0)
			return -1;
	/* load the global database */
	if(configure->prefs->basedir != NULL
			&& (filename = string_new_append(
					configure->prefs->basedir,
					"/data/" PACKAGE ".conf",
					NULL)) == NULL)
		return -configure_error(1, "%s", error_get(NULL));
	if(filename != NULL && config_load(configure->programs, filename) != 0)
		configure_warning(0, "%s: %s", filename,
			"Could not load program definitions");
	if(configure->prefs->basedir != NULL)
		string_delete(filename);
	/* load the database for the current system */
	if(configure->prefs->basedir != NULL)
		filename = string_new_append(configure->prefs->basedir,
				"/data/platform/", sHostOS[configure->os],
				".conf", NULL);
	else
		filename = string_new_append(DATADIR "/" PACKAGE "/platform/",
				sHostOS[configure->os], ".conf", NULL);
	if(filename == NULL)
		return -configure_error(1, "%s", error_get(NULL));
	else if(config_load(configure->programs, filename) != 0)
		configure_warning(0, "%s: %s", filename,
				"Could not load program definitions");
	string_delete(filename);
	return ret;
}

static int _configure_load(ConfigurePrefs * prefs, String const * directory,
		configArray * ca)
{
	int ret = 0;
	Config * config;
	String * path;
	String const * subdirs = NULL;

	if((path = string_new_append(directory, "/", PROJECT_CONF, NULL))
			== NULL)
		return configure_error(1, "%s", error_get(NULL));
	if((config = config_new()) == NULL)
	{
		string_delete(path);
		return configure_error(1, "%s", error_get(NULL));
	}
	config_set(config, "", "directory", directory);
	if(prefs->flags & PREFS_v)
		printf("%s%s%s", "Loading project file ", path, "\n");
	if(config_load(config, path) != 0)
		ret = error_print(PROGNAME);
	else
	{
		array_append(ca, &config);
		subdirs = config_get(config, "", "subdirs");
	}
	string_delete(path);
	if(subdirs == NULL)
		return ret;
	return _load_subdirs(prefs, directory, ca, subdirs);
}

static int _load_subdirs(ConfigurePrefs * prefs, char const * directory,
		configArray * ca, String const * subdirs)
{
	int ret = 0;
	int i;
	char c;
	String * subdir;
	String * p;

	if((subdir = string_new(subdirs)) == NULL)
		return 1;
	p = subdir;
	for(i = 0; ret == 0; i++)
	{
		if(subdir[i] != ',' && subdir[i] != '\0')
			continue;
		c = subdir[i];
		subdir[i] = '\0';
		ret = _load_subdirs_subdir(prefs, directory, ca, subdir);
		if(c == '\0')
			break;
		subdir += i + 1;
		i = 0;
	}
	string_delete(p);
	return ret;
}

static int _load_subdirs_subdir(ConfigurePrefs * prefs, char const * directory,
		configArray * ca, char const * subdir)
	/* FIXME error checking */
{
	int ret;
	String * p;

	p = string_new(directory);
	string_append(&p, "/");
	string_append(&p, subdir);
	ret = _configure_load(prefs, p, ca);
	string_delete(p);
	return ret;
}

static int _configure_do(Configure * configure, configArray * ca)
{
	size_t i;
	size_t cnt = array_count(ca);
	String const * di;
	size_t j;
	Config * cj;
	String const * dj;

	for(i = 0; i < cnt; i++)
	{
		array_get_copy(ca, i, &configure->project);
		if((di = config_get(configure->project, "", "directory"))
				== NULL)
			continue;
		for(j = i; j < cnt; j++)
		{
			array_get_copy(ca, j, &cj);
			if((dj = config_get(cj, "", "directory")) == NULL)
				continue;
			if(string_find(dj, di) == NULL)
				break;
		}
		if(makefile(configure, di, ca, i, j) != 0)
			break;
	}
	return (i == cnt) ? 0 : 1;
}


/* accessors */
/* configure_can_library_static */
int configure_can_library_static(Configure * configure)
{
	switch(configure->os)
	{
		case HO_WIN32:
			return 0;
		default:
			return 1;
	}
}


/* configure_get_config */
String const * configure_get_config(Configure * configure,
		String const * section, String const * variable)
{
	return config_get(configure->project, section, variable);
}


/* configure_get_exeext */
String const * configure_get_extension(Configure * configure,
		String const * extension)
{
	String const section[] = "extensions";

	return config_get(configure->programs, section, extension);
}


/* configure_get_os */
HostOS configure_get_os(Configure * configure)
{
	return configure->os;
}


/* configure_get_prefs */
ConfigurePrefs const * configure_get_prefs(Configure * configure)
{
	return configure->prefs;
}


/* configure_get_program */
String const * configure_get_program(Configure * configure, String const * name)
{
	String const section[] = "programs";
	String const * program;

	if((program = config_get(configure->programs, section, name)) != NULL)
		return program;
	return name;
}


/* configure_is_flag_set */
unsigned int configure_is_flag_set(Configure * configure, unsigned int flags)
{
	return (configure->prefs->flags & flags) ? 1 : 0;
}


/* source_extension */
String const * _source_extension(String const * source)
{
	size_t len;

	for(len = string_length(source); len > 0; len--)
		if(source[len - 1] == '.')
			return &source[len];
	return NULL;
}


/* source_type */
ObjectType _source_type(String const * source)
{
	String const * extension;
	size_t i;

	if((extension = _source_extension(source)) == NULL)
		extension = source;
	for(i = 0; sExtensionType[i].extension != NULL; i++)
		if(string_compare(sExtensionType[i].extension, extension) == 0)
			return sExtensionType[i].type;
	return OT_UNKNOWN;
}
