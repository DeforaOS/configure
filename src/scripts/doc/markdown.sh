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
CONFIGSH="${0%/markdown.sh}/../config.sh"
PREFIX="/usr/local"
PROGNAME="markdown.sh"
#executables
DEBUG="_debug"
INSTALL="install -m 0644"
MD2RST="m2r --overwrite"
MKDIR="mkdir -m 0755 -p"
RM="rm -f"
RST2HTML="rst2html5"
RST2MAN="rst2man"
RST2PDF="rst2pdf"

[ -f "$CONFIGSH" ] && . "$CONFIGSH"


#functions
#debug
_debug()
{
	echo "$@" 1>&3
	"$@"
}


#markdown
_markdown()
{
	res=2
	target="$1"

	source="${target%.*}.md"
	[ -f "$source" ] || source="${source#$OBJDIR}"
	ext="${target##*.}"
	ext="${ext##.}"
	case "$ext" in
		html)
			$DEBUG $MD2RST "$source"		|| return 2
			$DEBUG $RST2HTML "${target%.*}.rst" > "$target"
			res=$?
			;;
		pdf)
			$DEBUG $MD2RST "$source"		|| return 2
			R2P="$RST2PDF"
			[ -f "${source%.*}.style" ] &&
				R2P="$R2P -s ${source%.*}.style"
			$DEBUG $R2P "${target%.*}.rst" > "$target"
			res=$?
			;;
		rst)
			$DEBUG $MD2RST "$source"		|| return 2
			res=$?
			;;
		1|2|3|4|5|6|7|8|9)
			$DEBUG $MD2RST "$source"		|| return 2
			$DEBUG $RST2MAN "${target%.*}.rst" > "$target"
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
		return 2
	fi
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
[ -z "$MANDIR" ] && MANDIR="$DATADIR/man"

exec 3>&1
while [ $# -gt 0 ]; do
	target="$1"
	shift

	#determine the type
	ext="${target##*.}"
	ext="${ext##.}"
	case "$ext" in
		html|pdf)
			instdir="$DATADIR/doc/$ext/$PACKAGE"
			source="${target#$OBJDIR}"
			source="${source%.*}.md"
			;;
		1|2|3|4|5|6|7|8|9)
			instdir="$MANDIR/man$ext"
			;;
		*)
			_error "$target: Unknown type"
			exit 2
			;;
	esac

	#clean
	if [ "$clean" -ne 0 ]; then
		case "$ext" in
			html|pdf|1|2|3|4|5|6|7|8|9)
				tmpfile="${target#$OBJDIR}"
				tmpfile="${source%.*}.rst"
				$DEBUG $RM -- "$tmpfile"
				;;
			rst)
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
	#XXX ignore errors
	_markdown "$target"					|| break
done
