#!/bin/sh
#$Id$
#Copyright (c) 2019-2021 Pierre Pronchery <khorben@defora.org>
#
#Redistribution and use in source and binary forms, with or without
#modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
#FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
#DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
#OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.



#variables
CONFIGSH="${0%/template.sh}/../config.sh"
PREFIX="/usr/local"
DATABASEDIR="../data"
PROGNAME="template.sh"
#executables
DEBUG="_debug"
MKDIR="mkdir -p"
INSTALL="install -m 0644"
RM="rm -f"
SED="sed"

[ -f "$CONFIGSH" ] && . "$CONFIGSH"


#functions
#debug
_debug()
{
	echo "$@" 1>&3
	"$@"
}


#template
_template()
{
	res=2
	target="$1"

	ext="${target##*.}"
	subject="${target##*-}"
	subject="${subject%.*}"
	database="$DATABASEDIR/$subject.db"
	if [ "$subject" = "$target" -o "$subject" = "${target%.*}" ]; then
		_error "$target: No subject found for target"
		return $?
	fi
	if [ ! -f "$database" ]; then
		_error "$subject: No database found for subject"
		return $?
	fi
	source="${target%-*}.$ext.in"
	[ -f "$source" ] || source="${source#$OBJDIR}"
	if [ ! -f "$source" ]; then
		_error "$source: Could not find source"
		return $?
	fi
	case "$ext" in
		html|sgml|xml)
			options=
			while read line; do
				name="${line%%=*}"
				value="${line#*=}"
				value="$(echo "$value" | $SED \
					-e "s/[ $'\"\`\\\\]/\\\&/g" \
					-e "s/&/\\\\\\&amp;/g" \
					-e "s/\'/\\\\\\&apos;/g" \
					-e "s/\"/\\\\\\&quot;/g" \
					-e "s/</\\\\\\&lt;/g")"
				options="$options -e \"s/@@$name@@/$value/g\""
			done < "$database"
			eval $SED $options < "$source" > "$target"
			res=$?
			;;
		md|rst|txt)
			options=
			while read line; do
				name="${line%%=*}"
				value="${line#*=}"
				value="$(echo "$value" | $SED \
					-e "s/[ $'\"\`\\\\/]/\\\&/g")"
				options="$options -e \"s/@@$name@@/$value/g\""
			done < "$database"
			eval $SED $options < "$source" > "$target"
			res=$?
			;;
		*)
			_error "$target: Unknown type"
			return 2
			;;
	esac

	if [ $res -ne 0 ]; then
		_error "$target: Could not create target"
		$DEBUG $RM -- "$target"
	fi

	return $res
}


#error
_error()
{
	echo "$PROGNAME: $@" 1>&2
	return 2
}


#usage
_usage()
{
	echo "Usage: $PROGNAME [-c|-i|-u][-P prefix] target..." 1>&2
	return 1
}


#main
clean=0
install=0
uninstall=0
while getopts "ciO:uP:" name; do
	case "$name" in
		c)
			clean=1
			;;
		i)
			uninstall=0
			install=1
			;;
		O)
			export "${OPTARG%%=*}"="${OPTARG#*=}"
			;;
		u)
			install=0
			uninstall=1
			;;
		P)
			PREFIX="$OPTARG"
			;;
		?)
			_usage
			exit $?
			;;
	esac
done
shift $((OPTIND - 1))
if [ $# -lt 1 ]; then
	_usage
	exit $?
fi

#check the variables
if [ -z "$PACKAGE" ]; then
	_error "The PACKAGE variable needs to be set"
	exit $?
fi

[ -z "$DATADIR" ] && DATADIR="$PREFIX/share"

exec 3>&1
while [ $# -gt 0 ]; do
	target="$1"
	shift

	#determine the type
	ext="${target##*.}"
	case "$ext" in
		html|md|rst|sgml|txt|xml)
			instdir="$DATADIR/doc/$ext/$PACKAGE"
			source="${target#$OBJDIR}"
			source="${source%.*}.md"
			;;
		*)
			_error "$target: Unknown type"
			exit 2
			;;
	esac

	#clean
	if [ "$clean" -ne 0 ]; then
		case "$ext" in
			html|md|rst|sgml|txt|xml)
				;;
			*)
				_error "$target: Unknown type"
				exit 2
				;;
		esac
		exit $?
	fi

	#uninstall
	if [ "$uninstall" -eq 1 ]; then
		target="${target#$OBJDIR}"
		$DEBUG $RM -- "$instdir/$target"		|| exit 2
		continue
	fi

	#install
	if [ "$install" -eq 1 ]; then
		source="${target#$OBJDIR}"
		dirname=
		if [ "${source%/*}" != "$source" ]; then
			dirname="/${source%/*}"
		fi
		$DEBUG $MKDIR -- "$instdir$dirname"		|| exit 2
		$DEBUG $INSTALL "$target" "$instdir/$source"	|| exit 2
		continue
	fi

	#create
	_template "$target"					|| exit 2
done
