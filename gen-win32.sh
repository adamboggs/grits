#!/bin/bash
./autogen.sh \
	--enable-shared \
	--disable-static \
	--host=i686-pc-mingw32 \
	CFLAGS="-g -Werror -Wno-unused -O3"
