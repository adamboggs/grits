#!/bin/bash
dir=$(dirname $(readlink -f $0))
./autogen.sh \
	"--host=i686-pc-mingw32" \
	"--libdir=Z:$dir/src/.libs" \
	"--includedir=Z:$dir/include" \
	CFLAGS="-g -Werror -Wno-unused $CFLAGS"
