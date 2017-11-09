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



#ifndef CONFIGURE_CONFIGURE_H
# define CONFIGURE_CONFIGURE_H

# include <System.h>


/* types */
ARRAY(Config *, config)

typedef struct _EnumMap
{
	unsigned int value;
	String const * string;
} EnumMap;

typedef enum _HostArch
{
	HA_AMD64 = 0,
	HA_ARM,
	HA_I386, HA_I486, HA_I586, HA_I686,
	HA_SPARC, HA_SPARC64,
	HA_UNKNOWN
} HostArch;
# define HA_LAST	HA_UNKNOWN
# define HA_COUNT	(HA_LAST + 1)
extern const String * sHostArch[HA_COUNT];

typedef enum _HostOS
{
	HO_DEFORAOS = 0,
	HO_DARWIN,
	HO_GNU_LINUX,
	HO_FREEBSD, HO_NETBSD, HO_OPENBSD,
	HO_SUNOS,
	HO_WIN32,
	HO_UNKNOWN
} HostOS;
# define HO_LAST	HO_UNKNOWN
# define HO_COUNT	(HO_LAST + 1)
/* aliases */
# define HO_MACOSX	HO_DARWIN
extern const String * sHostOS[HO_COUNT];

typedef enum _HostKernel
{
	HK_LINUX20 = 0, HK_LINUX22, HK_LINUX24, HK_LINUX26,
	HK_MACOSX106, HK_MACOSX113,
	HK_NETBSD2, HK_NETBSD3, HK_NETBSD4, HK_NETBSD5, HK_NETBSD51,
	HK_OPENBSD40, HK_OPENBSD41,
	HK_SUNOS57, HK_SUNOS58, HK_SUNOS59, HK_SUNOS510,
	HK_UNKNOWN
} HostKernel;
# define HK_LAST	HK_UNKNOWN
# define HK_COUNT	(HK_LAST + 1)
typedef struct _HostKernelMap
{
	HostOS os;
	const char * version;
	const char * os_display;
	const char * version_display;
} HostKernelMap;
extern const HostKernelMap sHostKernel[HK_COUNT];

typedef enum _TargetType
{
	TT_BINARY = 0, TT_LIBRARY, TT_LIBTOOL, TT_OBJECT, TT_PLUGIN, TT_SCRIPT,
	TT_UNKNOWN
} TargetType;
# define TT_LAST	TT_UNKNOWN
# define TT_COUNT	(TT_LAST + 1)
extern const String * sTargetType[TT_COUNT];

typedef enum _ObjectType
{
	OT_C_SOURCE = 0,
	OT_CXX_SOURCE,
	OT_ASM_SOURCE,
	OT_OBJC_SOURCE,
	OT_OBJCXX_SOURCE,
	OT_VERILOG_SOURCE,
	OT_UNKNOWN
} ObjectType;
# define OT_LAST	OT_UNKNOWN
# define OT_COUNT	(OT_LAST + 1)
struct ExtensionType
{
	const char * extension;
	ObjectType type;
};
extern const struct ExtensionType * sExtensionType;


/* constants */
# define PROJECT_CONF	"project.conf"
# define MAKEFILE	"Makefile"


/* configure */
/* types */
typedef struct _ConfigurePrefs
{
	int flags;
	char * bindir;
	char * destdir;
	char * includedir;
	char * libdir;
	char * prefix;
	char * os;
	char * sbindir;
} ConfigurePrefs;
# define PREFS_n	0x1
# define PREFS_S	0x2
# define PREFS_v	0x4
typedef struct _Configure
{
	ConfigurePrefs * prefs;
	Config * config;
	HostArch arch;
	HostOS os;
	HostKernel kernel;
	struct
	{
		char const * exeext;
		char const * soext;
	} extensions;
	struct
	{
		char const * ar;
		char const * as;
		char const * cc;
		char const * ccshared;
		char const * cxx;
		char const * install;
		char const * libtool;
		char const * ln;
		char const * mkdir;
		char const * ranlib;
		char const * rm;
		char const * tar;
	} programs;
} Configure;


/* functions */
int configure(ConfigurePrefs * prefs, String const * directory);

/* accessors */
int configure_can_library_static(Configure * configure);

String const * configure_get_config(Configure * configure,
		String const * section, String const * variable);
String const * configure_get_exeext(Configure * configure);
HostOS configure_get_os(Configure * configure);
String const * configure_get_soext(Configure * configure);

unsigned int configure_is_flag_set(Configure * configure, unsigned int flags);

/* useful */
int configure_error(char const * message, int ret);

/* generic */
unsigned int enum_map_find(unsigned int last, EnumMap const * map,
		String const * str);
unsigned int enum_string(unsigned int last, const String * strings[],
		String const * str);
unsigned int enum_string_short(unsigned int last, const String * strings[],
		String const * str);

String const * _source_extension(String const * source);
ObjectType _source_type(String const * source);

#endif /* !CONFIGURE_CONFIGURE_H */
