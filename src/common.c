/* $Id$ */
/* Copyright (c) 2017-2020 Pierre Pronchery <khorben@defora.org> */
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



#include "common.h"


/* public */
/* functions */
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
					string_get_length(strings[i])) == 0)
			return i;
	return last;
}


/* source_extension */
String const * source_extension(String const * source)
{
	size_t len;

	for(len = string_get_length(source); len > 0; len--)
		if(source[len - 1] == '.')
			return &source[len];
	return NULL;
}


/* source_type */
ObjectType source_type(String const * source)
{
	String const * extension;
	size_t i;

	if((extension = source_extension(source)) == NULL)
		extension = source;
	for(i = 0; sExtensionType[i].extension != NULL; i++)
		if(string_compare(sExtensionType[i].extension, extension) == 0)
			return sExtensionType[i].type;
	return OT_UNKNOWN;
}
