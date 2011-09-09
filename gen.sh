#!/bin/bash
dir=$(dirname $(readlink -f $0))
./autogen.sh \
	"--enable-gtk-doc" \
	"--libdir=$dir/src/.libs" \
	"--includedir=$dir/include" \
	CFLAGS="-g -Werror $CFLAGS"
