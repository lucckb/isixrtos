#!/usr/local/bin/bash -e
source macbuild-common.env
#GCC base
arm-none-eabi-ld -v
compname="$BASEDIR/${pkg_dirs[gcc]}"

#if [ ! -f "$compname/.patched" ]; then
#		patch -p2 -d "$BASEDIR/${pkg_dirs[gcc]}" < patch-gcc-config-arm-t-arm-elf.diff
#		touch "$compname/.patched"
#fi

#Compile
if [ ! -f "$compname/.compile-full" ]; then
	#GCC base compile
	echo "Compile GCC ..."
	pushd "$BASEDIR"
	mkdir -p build-gcc && cd build-gcc
	../${pkg_dirs[gcc]}/configure \
		--target=$TARGET \
		--prefix=$PREFIX \
		--build=$CHOST \
		--host=$CHOST \
		--with-as=${PREFIX}/bin/${TARGET}-as \
		--with-ld=${PREFIX}/bin/${TARGET}-ld \
		--with-mpfr=/usr/local/Cellar/mpfr/4.1.0 \
		--with-mpc=/usr/local/Cellar/libmpc/1.2.1 \
		--with-gmp=/usr/local/Cellar/gmp/6.2.1 \
		--with-isl=/usr/local/Cellar/isl/0.24 \
		--with-libelf=/usr/local/Cellar/libelf/0.8.13_1 \
		--enable-languages=c,c++ \
        --enable-plugins \
        --disable-decimal-float \
        --disable-libffi \
        --disable-libgomp \
        --disable-libmudflap \
        --disable-libquadmath \
        --disable-libssp \
        --disable-libstdcxx-pch \
        --disable-nls \
        --disable-shared \
        --disable-threads \
        --disable-tls \
        --with-gnu-as \
        --with-gnu-ld \
        --with-newlib \
        --with-headers=yes \
		--with-multilib-list=rmprofile  

	make -j $(nproc) INHIBIT_LIBC_CFLAGS="-DUSE_TM_CLONE_REGISTRY=0"
	popd
	touch "$compname/.compile-full" 
fi


if [ ! -f "$compname/.installed-full" ]; then
	pushd "$BASEDIR"
	cd build-gcc
	make install
	popd
	touch "$compname/.installed-full" 
fi

