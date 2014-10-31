/* $Id$ */
/* Copyright (c) 2004-2014 Pierre Pronchery <khorben@defora.org> */
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "makefile.h"
#include "configure.h"
#include "../config.h"

#ifndef PROGNAME
# define PROGNAME PACKAGE
#endif


/* configure */
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
	{ "c",		OT_C_SOURCE	},
	{ "cpp",	OT_CXX_SOURCE	},
	{ "cxx",	OT_CXX_SOURCE	},
	{ "c++",	OT_CXX_SOURCE	},
	{ "asm",	OT_ASM_SOURCE	},
	{ "S",		OT_ASM_SOURCE	},
	{ NULL,		0		}
};
const struct ExtensionType * sExtensionType = _sExtensionType;


/* prototypes */
String const * _source_extension(String const * source);
ObjectType _source_type(String const * source);


/* functions */
/* configure_error */
int configure_error(char const * message, int ret)
{
	fputs(PROGNAME ": ", stderr);
	perror(message);
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
static void _configure_detect_programs(Configure * configure);
static int _configure_load(Prefs * prefs, char const * directory,
		configArray * ca);
static int _load_subdirs(Prefs * prefs, char const * directory,
		configArray * ca, String const * subdirs);
static int _load_subdirs_subdir(Prefs * prefs, char const * directory,
		configArray * ca, char const * subdir);
static int _configure_do(Configure * configure, configArray * ca);

int configure(Prefs * prefs, char const * directory)
{
	int ret;
	Configure cfgr;
	configArray * ca;
	int flags = prefs->flags;
	int i;
	Config * p;

	if((ca = configarray_new()) == NULL)
		return error_print(PROGNAME);
	cfgr.prefs = prefs;
	_configure_detect(&cfgr);
	_configure_detect_programs(&cfgr);
	if((ret = _configure_load(prefs, directory, ca)) == 0)
	{
		if(prefs->flags & PREFS_n)
			ret = _configure_do(&cfgr, ca);
		else
		{
			prefs->flags = PREFS_n;
			if(_configure_do(&cfgr, ca) == 0)
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
		configure_error("system detection failed", 0);
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

static void _configure_detect_programs(Configure * configure)
{
	configure->programs.ar = "ar";
	configure->programs.as = "as";
	configure->programs.cc = "cc";
	configure->programs.ccshared = "$(CC) -shared";
	configure->programs.cxx = "c++";
	configure->programs.install = "install";
	configure->programs.libtool = "libtool";
	configure->programs.ln = "ln -f";
	configure->programs.mkdir = "mkdir -m 0755 -p";
	configure->programs.ranlib = "ranlib";
	configure->programs.rm = "rm -f";
	configure->programs.tar = "tar";
	/* platform-specific */
	switch(configure->os)
	{
		case HO_WIN32:
			configure->programs.ccshared = "$(CC) -shared"
				" -Wl,-no-undefined"
				" -Wl,--enable-runtime-pseudo-reloc";
			break;
		default:
			break;
	}
}

static int _configure_load(Prefs * prefs, String const * directory,
		configArray * ca)
{
	int ret = 0;
	Config * config;
	String * path;
	String const * subdirs = NULL;

	if((path = string_new(directory)) == NULL)
		return configure_error(directory, 1);
	if(string_append(&path, "/") != 0
			|| string_append(&path, PROJECT_CONF) != 0
			|| (config = config_new()) == NULL)
	{
		string_delete(path);
		return configure_error(directory, 1);
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

static int _load_subdirs(Prefs * prefs, char const * directory,
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

static int _load_subdirs_subdir(Prefs * prefs, char const * directory,
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
		array_get_copy(ca, i, &configure->config);
		if((di = config_get(configure->config, "", "directory"))
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


/* configure_get_config */
String const * configure_get_config(Configure * configure,
		String const * section, String const * variable)
{
	return config_get(configure->config, section, variable);
}


/* configure_get_soext */
String const * configure_get_soext(Configure * configure)
{
	if(configure->os == HO_MACOSX)
		return ".dylib";
	else if(configure->os == HO_WIN32)
		return ".dll";
	return ".so";
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
