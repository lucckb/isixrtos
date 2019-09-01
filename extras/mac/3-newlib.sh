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
			--enable-interwork \
			--with-gnu-ld \
			--with-gnu-as \
			--disable-shared \
			--disable-newlib-supplied-syscalls \
			--enable-newlib-reent-small \
			--disable-newlib-fvwrite-in-streamio \
			--disable-newlib-fseek-optimization \
			--disable-newlib-wide-orient \
			--enable-newlib-nano-malloc \
			--disable-newlib-unbuf-stream-opt \
			--enable-target-optspace \
			--enable-newlib-io-float \
			--disable-newlib-fvwrite-in-streamio \
			--disable-newlib-wide-orient \
			--enable-newlib-nano-malloc \
			--disable-newlib-unbuf-stream-opt \
			--enable-newlib-nano-formatted-io \
			--enable-lto \
			--disable-werror \
			--disable-libgloss
		make
		popd
		touch "$compname/.compiled" 
	fi
	if [ ! -f "$compname/.installed" ]; then
		pushd $compname
		sudo make install
		popd
		touch "$compname/.installed" 
	fi
}

#Build
build

