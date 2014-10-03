/* $Id$ */
/* Copyright (c) 2014 Pierre Pronchery <khorben@defora.org> */
/* This file is part of DeforaOS Devel configure */
/* This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */



#include <sys/stat.h>
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
/* prototypes */
static void _prefs_init(Prefs * prefs);

static int _usage(void);


/* functions */
/* prefs_init */
static void _prefs_init(Prefs * prefs)
{
	struct stat st;

	memset(prefs, 0, sizeof(*prefs));
	prefs->destdir = "";
	if(stat("/Apps", &st) == 0)
	{
		prefs->bindir = "Binaries";
		prefs->includedir = "Includes";
		prefs->libdir = "Libraries";
		/* XXX needs auto-detection for the sub-directory */
		prefs->prefix = "/Apps";
		prefs->sbindir = "Binaries";
	}
	else
	{
		prefs->bindir = "bin";
		prefs->includedir = "include";
		prefs->libdir = "lib";
		prefs->prefix = "/usr/local";
		prefs->sbindir = "sbin";
	}
}


/* usage */
static int _usage(void)
{
	Prefs prefs;

	_prefs_init(&prefs);
	fprintf(stderr, "%s%s%s%s%s%s%s%s%s%s%s",
"Usage: configure [-nvS][options...][directory...]\n"
"  -n	Do not actually write Makefiles\n"
"  -v	Verbose mode\n"
"  -b	Binary files directory (default: \"", prefs.bindir, "\")\n"
"  -d	Destination prefix (default: \"\")\n"
"  -i	Include files directory (default: \"", prefs.includedir, "\")\n"
"  -l	Library files directory (default: \"", prefs.libdir, "\")\n"
"  -O	Force Operating System (default: auto-detected)\n"
"  -p	Installation directory prefix (default: \"", prefs.prefix, "\")\n"
"  -S	Warn about potential security risks\n"
"  -s	Super-user executable files directory (default: \"", prefs.sbindir,
"\")\n");
	return 1;
}


/* public */
/* functions */
/* main */
int main(int argc, char * argv[])
{
	int ret = 0;
	Prefs prefs;
	int o;

	_prefs_init(&prefs);
	while((o = getopt(argc, argv, "d:i:l:nO:p:Ss:v")) != -1)
		switch(o)
		{
			case 'b':
				prefs.bindir = optarg;
				break;
			case 'd':
				prefs.destdir = optarg;
				break;
			case 'i':
				prefs.includedir = optarg;
				break;
			case 'l':
				prefs.libdir = optarg;
				break;
			case 'n':
				prefs.flags |= PREFS_n;
				break;
			case 'O':
				prefs.os = optarg;
				break;
			case 'p':
				prefs.prefix = optarg;
				break;
			case 'S':
				prefs.flags |= PREFS_S;
				break;
			case 's':
				prefs.sbindir = optarg;
				break;
			case 'v':
				prefs.flags |= PREFS_v;
				break;
			case '?':
				return _usage();
		}
	if(optind == argc)
		return configure(&prefs, ".");
	for(; optind < argc; optind++)
		ret |= configure(&prefs, argv[optind]);
	return (ret == 0) ? 0 : 2;
}
