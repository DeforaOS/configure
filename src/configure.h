/* $Id$ */
/* Copyright (c) 2006-2021 Pierre Pronchery <khorben@defora.org> */
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
ARRAY2(Config *, config)

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
	TT_BINARY = 0, TT_COMMAND, TT_LIBRARY, TT_LIBTOOL, TT_OBJECT, TT_PLUGIN,
	TT_SCRIPT, TT_UNKNOWN
} TargetType;
# define TT_LAST	TT_UNKNOWN
# define TT_COUNT	(TT_LAST + 1)
extern const String * sTargetType[TT_COUNT];

typedef enum _ObjectType
{
	OT_C_SOURCE = 0,
	OT_CXX_SOURCE,
	OT_ASM_SOURCE,
	OT_ASMPP_SOURCE,
	OT_JAVA_SOURCE,
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
# ifndef CONFEXT
#  define CONFEXT	".conf"
# endif
# ifndef PROJECT_CONF
#  define PROJECT_CONF	"project" CONFEXT
# endif
# ifndef MAKEFILE
#  define MAKEFILE	"Makefile"
# endif


/* configure */
/* types */
typedef struct _ConfigurePrefs
{
	unsigned int flags;
	String const * mode;
	String const * os;
} ConfigurePrefs;
# define PREFS_n	0x1
# define PREFS_S	0x2
# define PREFS_v	0x4

typedef struct _Configure Configure;


/* functions */
Configure * configure_new(ConfigurePrefs * prefs);
void configure_delete(Configure * configure);

/* accessors */
int configure_can_library_static(Configure * configure);

String const * configure_get_config(Configure * configure,
		String const * section, String const * variable);
String const * configure_get_config_mode(Configure * configure,
		String const * mode, String const * variable);
String const * configure_get_extension(Configure * configure,
		String const * extension);
String const * configure_get_mode(Configure * configure);
String const * configure_get_mode_title(Configure * configure,
		String const * mode);
String const * configure_get_path(Configure * configure, String const * path);
HostOS configure_get_os(Configure * configure);
ConfigurePrefs const * configure_get_prefs(Configure * configure);
String const * configure_get_program(Configure * configure,
		String const * name);

unsigned int configure_is_flag_set(Configure * configure, unsigned int flags);

int configure_set_mode(Configure * configure, String const * mode);
int configure_set_path(Configure * configure, String const * path,
		String const * value);

/* useful */
int configure_error(int ret, char const * format, ...);
int configure_warning(int ret, char const * format, ...);

int configure_project(Configure * configure, String const * directory);

#endif /* !CONFIGURE_CONFIGURE_H */
