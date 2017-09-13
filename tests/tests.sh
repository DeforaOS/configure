#!/bin/sh
#$Id$
#Copyright (c) 2014-2017 Pierre Pronchery <khorben@defora.org>
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
CONFIGURE="$OBJDIR../src/configure"
DATE="date"
DIFF="diff"
RM="rm -f"


#functions
#fail
_fail()
{
	_test "$@"
	return 0
}


#test
_test()
{
	[ $# -eq 2 ]						|| return 2
	system="$1"
	subdir="$2"

	echo
	echo "Testing: $system $subdir"
	$CONFIGURE -O "$system" -- "$subdir" 2>&1
	res=$?
	if [ $res -eq 0 ]; then
		$DIFF -- "$subdir/Makefile.$system" "$subdir/Makefile" 2>&1
		res=$?
		$RM -- "$subdir/Makefile"
	fi
	if [ $res -ne 0 ]; then
		echo "$system $subdir: Failed with error code $res"
		echo "$system $subdir: FAIL" 1>&2
		return 2
	fi
	echo "$system $subdir: PASS" 1>&2
}


_tests()
{
	ret=0

	$DATE
	#XXX cross-compiling
	[ -n "$PKG_CONFIG_SYSROOT_DIR" ] && return 0
	_test "Darwin" "binary"					|| ret=2
	_test "Darwin" "library"				|| ret=2
	_test "Darwin" "libtool"				|| ret=2
	_test "Darwin" "plugin"					|| ret=2
	_test "DeforaOS" "binary"				|| ret=2
	_test "Linux" "library"					|| ret=2
	_test "Linux" "libtool"					|| ret=2
	_test "NetBSD" "binary"					|| ret=2
	_test "NetBSD" "include"				|| ret=2
	_test "NetBSD" "library"				|| ret=2
	_test "NetBSD" "libtool"				|| ret=2
	_test "NetBSD" "object"					|| ret=2
	_test "NetBSD" "package"				|| ret=2
	_test "NetBSD" "plugin"					|| ret=2
	_test "NetBSD" "script"					|| ret=2
	_test "NetBSD" "verilog"				|| ret=2
	_test "Windows" "binary"				|| ret=2
	_test "Windows" "library"				|| ret=2
	_test "Windows" "libtool"				|| ret=2
	return $ret
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

while [ $# -gt 0 ]; do
	target="$1"
	shift

	_tests > "$target"					|| exit 2
done
