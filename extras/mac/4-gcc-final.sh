#!/usr/local/bin/bash -e
source macbuild-common.env
#GCC base
arm-none-eabi-ld -v
compname="$BASEDIR/${pkg_dirs[gcc]}"

if [ ! -f "$compname/.patched" ]; then
		patch -p2 -d "$BASEDIR/${pkg_dirs[gcc]}" < patch-gcc-config-arm-t-arm-elf.diff
		touch "$compname/.patched"
fi

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
		--enable-multilib \
		--enable-languages="c,c++" \
		--disable-shared \
		--disable-threads \
		--disable-clocale \
		--disable-libstdcxx-time \
		--disable-libstdcxx-threads \
		--with-gnu-as \
		--with-gnu-ld \
		--with-newlib \
		--disable-libgomp \
		--disable-libmudflap \
		--disable-libssp \
		--disable-nls \
		--enable-lto \
		--disable-libstdcxx-verbose \
		--disable-libstdcxx-dual-abi \
		--disable-wchar_t  \
		--disable-vtable-verify \
		--enable-cxx-flags='-fomit-frame-pointer -ffunction-sections -fdata-sections' \
		--with-mpfr=/usr/local/Cellar/mpfr/4.0.2 \
		--with-mpc=/usr/local/Cellar/libmpc/1.1.0 \
		--with-gmp=/usr/local/Cellar/gmp/6.1.2_2
	make all 	
	popd
	touch "$compname/.compile-full" 
fi


if [ ! -f "$compname/.installed-full" ]; then
	pushd "$BASEDIR"
	cd build-gcc
	sudo make install
	popd
	touch "$compname/.installed-full" 
fi

