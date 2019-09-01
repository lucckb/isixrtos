#!/usr/local/bin/bash -e
source macbuild-common.env
#GCC base
arm-none-eabi-ld -v
compname="$BASEDIR/${pkg_dirs[gccbase]}"

if [ ! -f "$compname/.patched" ]; then
		patch -p2 -d "$BASEDIR/${pkg_dirs[gccbase]}" < patch-gcc-config-arm-t-arm-elf.diff
		touch "$compname/.patched"
fi

#Compile
if [ ! -f "$compname/.compile-base" ]; then
	#GCC base compile
	echo "Compile BASE GCC ..."
	pushd "$BASEDIR"
	mkdir -p base-gcc && cd base-gcc
	$compname/configure \
	--target=$TARGET \
	--prefix=$PREFIX \
	--build=$CHOST \
	--host=$CHOST \
	--with-as=${PREFIX}/bin/${TARGET}-as \
	--with-ld=${PREFIX}/bin/${TARGET}-ld \
	--enable-interwork \
	--enable-multilib \
	--enable-languages="c,c++" \
	--without-headers \
	--disable-shared \
	--disable-threads \
	--with-gnu-as \
	--with-gnu-ld \
	--with-newlib \
	--disable-libgomp \
	--disable-libmudflap \
	--disable-libssp \
	--disable-nls \
	--enable-lto \
	--with-mpfr=/usr/local/Cellar/mpfr/4.0.2 \
	--with-mpc=/usr/local/Cellar/libmpc/1.1.0 \
	--with-gmp=/usr/local/Cellar/gmp/6.1.2_2
	make all-gcc 
	popd
	touch "$compname/.compile-base" 
fi


if [ ! -f "$compname/.installed-base" ]; then
	pushd "$BASEDIR"
	cd base-gcc
	sudo make install-gcc
	popd
	touch "$compname/.installed-base" 
fi

