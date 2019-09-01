#!/usr/local/bin/bash -e
source macbuild-common.env

build() {
	#Binutils
	compname="$BASEDIR/${pkg_dirs[gdb]}"
	if [ ! -f "$compname/.compiled" ]; then
		echo "Compile binutils..."
		pushd $BASEDIR
		mkdir -p build-gdb && cd build-gdb
		$compname/configure \
			--target=$TARGET \
			--prefix=$PREFIX \
			--build=$CHOST \
			--host=$CHOST \
			--disable-nls \
			--enable-interwork \
			--enable-multilib \
			--with-float=soft  \
			--disable-shared \
			--disable-libgomp \
			--disable-libmudflap \
			--disable-libssp \
			--disable-werror \
			--enable-lto \
			--without-auto-load-safe-path \
			--with-guile=no \
			--with-readline=/usr/local//Cellar/readline/8.0.0_1
		make
		popd
		touch "$compname/.compiled" 
	fi
	if [ ! -f "$compname/.installed" ]; then
		pushd $BASEDIR
		cd build-gdb
		sudo make install
		popd
		touch "$compname/.installed" 
	fi
}

#Build
build

