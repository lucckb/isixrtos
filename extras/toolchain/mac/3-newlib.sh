#!/usr/local/bin/bash -e
source macbuild-common.env

build() {
	#Binutils
	compname="$BASEDIR/${pkg_dirs[newlib]}"
	if [ ! -f "$compname/.compiled" ]; then
		echo "Compile binutils..."
		pushd $compname
		./configure \
			--target=$TARGET \
			--prefix=$PREFIX \
			--build=$CHOST \
			--host=$CHOST \
			--disable-newlib-supplied-syscalls    \
			--enable-newlib-reent-check-verify    \
			--enable-newlib-reent-small           \
			--enable-newlib-retargetable-locking  \
			--disable-newlib-fvwrite-in-streamio  \
			--disable-newlib-fseek-optimization   \
			--disable-newlib-wide-orient          \
			--enable-newlib-nano-malloc           \
			--disable-newlib-unbuf-stream-opt     \
			--enable-lite-exit                    \
			--enable-newlib-global-atexit         \
			--enable-newlib-nano-formatted-io     \
			--disable-nls
		make
		popd
		touch "$compname/.compiled" 
	fi
	if [ ! -f "$compname/.installed" ]; then
		pushd $compname
		make install
		popd
		touch "$compname/.installed" 
	fi
}

#Build
build

