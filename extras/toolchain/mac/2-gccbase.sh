#!/usr/local/bin/bash -e
source macbuild-common.env
#GCC base
arm-none-eabi-ld -v
compname="$BASEDIR/${pkg_dirs[gccbase]}"

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
	--with-mpfr=/usr/local/Cellar/mpfr/4.1.0 \
	--with-mpc=/usr/local/Cellar/libmpc/1.2.1 \
	--with-gmp=/usr/local/Cellar/gmp/6.2.1 \
	--with-isl=/usr/local/Cellar/isl/0.24 \
	--with-libelf=/usr/local/Cellar/libelf/0.8.13_1 \
 	--enable-languages=c \
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
    --with-newlib \
    --without-headers \
    --with-gnu-as \
    --with-gnu-ld \
	--with-multilib-list=rmprofile  


	make all-gcc 
	popd
	touch "$compname/.compile-base" 
fi


if [ ! -f "$compname/.installed-base" ]; then
	pushd "$BASEDIR"
	cd base-gcc
	make install-gcc
	popd
	touch "$compname/.installed-base" 
fi

