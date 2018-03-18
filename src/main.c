/* $Id$ */
/* Copyright (c) 2014-2018 Pierre Pronchery <khorben@defora.org> */
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



#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "configure.h"
#include "../config.h"

#ifndef PROGNAME
# define PROGNAME PACKAGE
#endif


/* configure */
/* private */
/* types */
typedef struct _ConfigurePaths
{
	char const * bindir;
	char const * destdir;
	char const * includedir;
	char const * libdir;
	char const * prefix;
	char const * sbindir;
} ConfigurePaths;


/* prototypes */
static int _configure_set_paths(Configure * configure, ConfigurePaths * paths);

static int _usage(void);


/* functions */
/* configure_set_paths */
static int _configure_set_paths(Configure * configure, ConfigurePaths * paths)
{
	int ret = 0;

	if(paths->prefix != NULL)
		ret |= configure_set_path(configure, "prefix", paths->prefix);
	if(paths->bindir != NULL)
		ret |= configure_set_path(configure, "bindir", paths->bindir);
	if(paths->includedir != NULL)
		ret |= configure_set_path(configure, "includedir",
				paths->includedir);
	if(paths->libdir != NULL)
		ret |= configure_set_path(configure, "libdir", paths->libdir);
	if(paths->sbindir != NULL)
		ret |= configure_set_path(configure, "sbindir", paths->sbindir);
	return ret;
}


/* usage */
static String const * _usage_get_path(Configure * configure,
		String const * path);

static int _usage(void)
{
	Configure * configure;

	if((configure = configure_new(NULL)) == NULL)
		return configure_error(2, "%s", error_get(NULL));
	fprintf(stderr, "%s%s%s%s%s%s%s%s%s%s%s%s%s",
"Usage: " PROGNAME " [-nSv][-b bindir][-d destdir][-i includedir][-l libdir]\n"
"                 [-O system][-p prefix][-s sbindir][directory...]\n"
"  -n	Do not actually write Makefiles\n"
"  -v	Verbose mode\n"
"  -b	Binary files directory (default: \"",
			_usage_get_path(configure, "bindir"), "\")\n"
"  -d	Destination prefix (default: \"",
			_usage_get_path(configure, "destdir"),"\")\n"
"  -i	Include files directory (default: \"",
			_usage_get_path(configure, "includedir"), "\")\n"
"  -l	Library files directory (default: \"",
			_usage_get_path(configure, "libdir"), "\")\n"
"  -O	Force Operating System (default: auto-detected)\n"
"  -p	Installation directory prefix (default: \"",
			_usage_get_path(configure, "prefix"), "\")\n"
"  -S	Warn about potential security risks\n"
"  -s	Super-user executable files directory (default: \"",
			_usage_get_path(configure, "sbindir"),
"\")\n");
	configure_delete(configure);
	return 1;
}

static String const * _usage_get_path(Configure * configure,
		String const * path)
{
	String const * ret;

	if((ret = configure_get_path(configure, path)) == NULL)
		return "";
	return ret;
}


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	int ret = 0;
	ConfigurePrefs prefs;
	ConfigurePaths paths;
	int o;
	Configure * configure;

	memset(&prefs, 0, sizeof(prefs));
	memset(&paths, 0, sizeof(paths));
	while((o = getopt(argc, argv, "b:d:i:l:nO:p:qSs:v")) != -1)
		switch(o)
		{
			case 'b':
				paths.bindir = optarg;
				break;
			case 'd':
				paths.destdir = optarg;
				break;
			case 'i':
				paths.includedir = optarg;
				break;
			case 'l':
				paths.libdir = optarg;
				break;
			case 'n':
				prefs.flags |= PREFS_n;
				break;
			case 'O':
				prefs.os = optarg;
				break;
			case 'p':
				paths.prefix = optarg;
				break;
			case 'S':
				prefs.flags |= PREFS_S;
				break;
			case 's':
				paths.sbindir = optarg;
				break;
			case 'v':
				prefs.flags |= PREFS_v;
				break;
			case '?':
			default:
				return _usage();
		}
	if((configure = configure_new(&prefs)) == NULL)
		return configure_error(2, "%s", error_get(NULL));
	if(_configure_set_paths(configure, &paths) != 0)
		ret = configure_error(2, "%s", error_get(NULL));
	else if(optind == argc)
	{
		if((ret = configure_project(configure, ".")) != 0)
			configure_error(2, "%s", error_get(NULL));
	}
	else
		for(; optind < argc; optind++)
			if(configure_project(configure, argv[optind]) != 0)
				ret |= configure_error(2, "%s",
						error_get(NULL));
	configure_delete(configure);
	return (ret == 0) ? 0 : 2;
}
