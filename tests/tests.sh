#!/bin/sh
#$Id$
#Copyright (c) 2014-2022 Pierre Pronchery <khorben@defora.org>
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
PROGNAME="tests.sh"
#executables
CAT="cat"
CONFIGURE="$OBJDIR../tools/configure -S -q -v"
DATE="date"
DIFF="diff"
MKDIR="mkdir -p"
RM="rm -f"


#functions
#fail
_fail()
{
	_makefile_test "$@"
	return 0
}


#tests_date
_tests_date()
{
	$DATE
}


#tests_makefile
_tests_makefile()
{
	ret=0

	#XXX cross-compiling
	[ -n "$PKG_CONFIG_SYSROOT_DIR" ] && return 0
	_makefile_test "Darwin" "binary"			|| ret=2
	_makefile_test "Darwin" "library"			|| ret=2
	_makefile_test "Darwin" "libtool"			|| ret=2
	_makefile_test "Darwin" "plugin"			|| ret=2
	_makefile_test "DeforaOS" "binary"			|| ret=2
	_makefile_test "Linux" "library"			|| ret=2
	_makefile_test "Linux" "libtool"			|| ret=2
	_makefile_test "NetBSD" "binary"			|| ret=2
	_makefile_test "NetBSD" "command"			|| ret=2
	_makefile_test "NetBSD" "include"			|| ret=2
	_makefile_test "NetBSD" "golang"			|| ret=2
	_makefile_test "NetBSD" "java"				|| ret=2
	_makefile_test "NetBSD" "library"			|| ret=2
	_makefile_test "NetBSD" "libtool"			|| ret=2
	_makefile_test "NetBSD" "object"			|| ret=2
	_makefile_test "NetBSD" "package"			|| ret=2
	_makefile_test "NetBSD" "plugin"			|| ret=2
	_makefile_test "NetBSD" "script"			|| ret=2
	_makefile_test "NetBSD" "verilog"			|| ret=2
	_makefile_test "Windows" "binary"			|| ret=2
	_makefile_test "Windows" "library"			|| ret=2
	_makefile_test "Windows" "libtool"			|| ret=2
	return $ret
}

_makefile_test()
{
	[ $# -eq 2 ]						|| return 2
	system="$1"
	subdir="$2"
	config="ent h sh"

	echo
	echo "Testing: $system $subdir"
	$CONFIGURE -O "$system" -- "$subdir" 2>&1
	res=$?
	if [ $res -eq 0 ]; then
		$DIFF -- "$subdir/Makefile.$system" "$subdir/Makefile" 2>&1
		res=$?
		$RM -- "$subdir/Makefile"
	fi
	for ext in $config; do
		if [ $res -eq 0 -a -f "$subdir/config.$ext" ]; then
			$DIFF -- "$subdir/config.$ext.$system" \
				"$subdir/config.$ext" 2>&1
			res=$?
			$RM -- "$subdir/config.$ext"
		fi
	done
	if [ $res -ne 0 ]; then
		echo "$system $subdir: Failed with error code $res"
		echo "$system $subdir: FAIL" 1>&2
		return 2
	fi
	echo "$system $subdir: PASS"
	echo "$system $subdir: PASS" 1>&2
}


#tests_scripts
_tests_scripts()
{
	_scripts_test "../src/scripts/data/pkgconfig.sh" \
		"${OBJDIR}scripts/data/pkgconfig"
	_scripts_test "../src/scripts/tools/subst.sh" \
		"${OBJDIR}scripts/tools/subst"
}

_scripts_test()
{
	script="$1"
	target="$2"
	shift
	shift

	echo
	echo "Testing: $script"
	$MKDIR "$OBJDIR${target%/*}"
	$script "$target" "$@" &&
		$CAT "$target"
	res=$?
	if [ $res -ne 0 ]; then
		echo "$script: Failed with error code $res"
		echo "$script: FAIL" 1>&2
		return 2
	fi
	echo "$script: PASS"
	echo "$script: PASS" 1>&2
}


#usage
_usage()
{
	echo "Usage: $PROGNAME [-c] target..." 1>&2
	return 1
}


#main
clean=0
while getopts "cO:P:" name; do
	case "$name" in
		c)
			clean=1
			;;
		O)
			export "${OPTARG%%=*}"="${OPTARG#*=}"
			;;
		P)
			#ignored
			;;
		?)
			_usage
			exit $?
			;;
	esac
done
shift $(($OPTIND - 1))
if [ $# -lt 1 ]; then
	_usage
	exit $?
fi

[ "$clean" -ne 0 ] && exit 0

ret=0
while [ $# -gt 0 ]; do
	target="$1"
	shift

	(ret=0
	_tests_date
	_tests_makefile						|| ret=2
	_tests_scripts						|| ret=2
	exit $ret) > "$target"					|| exit 2
done
exit $ret
