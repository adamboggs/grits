#!/bin/bash
dir=$(dirname $(readlink -f $0))
./autogen.sh \
	"--enable-gtk-doc" \
	"--libdir=$dir/src/.libs" \
	"--includedir=$dir/src" \
	CFLAGS="-g -Werror -Wno-unused $CFLAGS" \
	LDFLAGS="-Wl,-z,defs"
