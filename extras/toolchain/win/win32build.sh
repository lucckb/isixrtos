#!/bin/bash -e



#Host architecture where it is build
BUILD=x86_64-unknown-linux
#For which target
TARGET=arm-none-eabi



if [ -z $1 ]; then 
	echo "32 or 64 bit not specified"
	exit -1
fi
case $1 in
	"32") 
		HOST=i686-w64-mingw32
	;;
	"64")
		HOST=x86_64-w64-mingw32
	;;
	*)
		echo "invalid target bits"
		exit -1
esac
NBITS=$1

#Basename for binary
BASENAME="arm-boff-mingw$NBITS-toolchain"

#Parallel build
export MAKEFLAGS="-j8"


#Library versions
BINUTILS_VER="2.32"
GCC_VER="9.2.0"
NEWLIB_VER="3.1.0"
GDB_VER="8.3.1"
EXPAT_VER="2.2.8"


#Downloads URL
declare -A dl_urls
dl_urls=(
		  [binutils]="http://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VER.tar.gz" \
		  [gcc]="ftp://ftp.gwdg.de/pub/misc/gcc/releases/gcc-$GCC_VER/gcc-$GCC_VER.tar.gz" \
		  [newlib]="ftp://sourceware.org/pub/newlib/newlib-$NEWLIB_VER.tar.gz" \
		  [gdb]="http://ftp.gnu.org/gnu/gdb/gdb-$GDB_VER.tar.xz" \
		  [expat]="http://downloads.sourceforge.net/project/expat/expat/$EXPAT_VER/expat-$EXPAT_VER.tar.bz2" \
		)


#BASE build path
BASEDIR="$(pwd)/win32_tmpdir"
DLDIR="$BASEDIR/dl"
BPREFIX=$BASEDIR/usr
PREFIX="$BASEDIR/$BASENAME"
export PATH=$PATH:$BPREFIX/bin
NEWLIBPATH=$(pwd)/newlib



declare -A dl_names
declare -A pkg_dirs
for index in ${!dl_urls[*]}
do
	name="$(basename ${dl_urls[$index]})" 
	namene=${name%.*}
	namene=${namene%.*}
	dl_names+=( [$index]="$name" )
	pkg_dirs+=( [$index]="$namene" )
done


download() {
echo "Num files for download: ${#dl_urls[*]}"
for item in ${dl_urls[*]}
do
	local bn=$(basename $item)
	if [ ! -f "$DLDIR/$bn" ]; then
		printf " Downloading %s\n" $bn
		wget -P $DLDIR $item
	fi
done
}

unpackall() {
for index in ${!dl_names[*]}
do
	local item="${dl_names[$index]}" 
	local diri="${pkg_dirs[$index]}" 
	if [ ! -d "$BASEDIR/$diri" ]; then
		echo "Unpacking $item ..."
		tar xf "$DLDIR/$item" -C $BASEDIR
	fi
done
}


compile() {
	local f="$BASEDIR/${pkg_dirs[$1]}"
	if [ ! -f "$f/.installed" ]; then
		pushd $f
		./configure $2
		if [ -z $3 ]; then
			make
		else 
			make $3 
		fi
		local save=$MAKEFLAGS
		export MAKEFLAGS="-j1"
		if [ -z $4 ]; then
			make
		else
			make $4
		fi
		export MAKEFLAGS=$save
		popd
		touch "$f/.installed"
	fi
}

build() {
	mkdir -p $PREFIX
	#EXPAT
	echo "Compile Expat ..."
	compile expat \
	" --build=$BUILD --host=$HOST  --prefix=$BPREFIX --enable-static --disable-shared" \
		"" \
		install
	#Binutils
	echo "Compile binutils..."
	compile binutils \
	"--target=$TARGET --host=$HOST --build=$BUILD --prefix=$PREFIX --enable-interwork --enable-multilib --disable-nls --disable-werror --enable-lto" \
		"" \
		install
	#Newlib
	echo "Compile newlib..."
	compile newlib \
		"--target=$TARGET --host=$HOST --build=$BUILD --prefix=$PREFIX  --enable-interwork \
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
		--disable-newlib-io-float \
		--disable-newlib-fvwrite-in-streamio \
		--disable-newlib-wide-orient \
		--enable-newlib-nano-malloc \
		--disable-newlib-unbuf-stream-opt \
		--enable-newlib-nano-formatted-io \
        --with-gnu-ld \
        --with-gnu-as \
        --enable-lto" \
		'' \
		install
	
	#GCC patch it
	if [ ! -f "$BASEDIR/${pkg_dirs[gcc]}/.patched" ]; then
		patch -p2 -d "$BASEDIR/${pkg_dirs[gcc]}" < patch-gcc-config-arm-t-arm-elf.diff
		touch "$BASEDIR/${pkg_dirs[gcc]}/.patched"
	fi

	if [ ! -f "$BASEDIR/${pkg_dirs[gcc]}/.installed" ]; then
		#GCC compile
		echo "Compile GCC ..."
		pushd "$BASEDIR"
		mkdir build-gcc && cd build-gcc
		../${pkg_dirs[gcc]}/configure \
		--build=$BUILD --target=$TARGET \
		--host=$HOST --prefix=$PREFIX \
		--enable-interwork \
		--enable-multilib \
		--enable-languages="c,c++,lto" \
		--disable-shared \
		--enable-static \
		--disable-threads \
		--disable-clocale \
		--disable-libstdcxx-time \
		--disable-libstdcxx-threads \
		--with-float=soft \
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
		--disable-werror
		make all 
		make install
		popd
		touch "$BASEDIR/${pkg_dirs[gcc]}/.installed" 
	fi

	#Prepare GDB 
	if [ ! -f "$BASEDIR/${pkg_dirs[gdb]}/.installed" ]; then
		pushd "$BASEDIR"
		mkdir build-gdb && cd build-gdb
		../${pkg_dirs[gdb]}/configure \
			-build=$BUILD --target=$TARGET --host=$HOST --prefix=$PREFIX \
			--enable-interwork \
			--enable-multilib \
			--disable-libgomp \
			--disable-libmudflap \
			--disable-libssp \
			--disable-werror \
			--disable-shared \
			--enable-static \
			--enable-lto \
			--without-auto-load-safe-path \
			--with-expat \
			--with-libexpat-prefix=$BPREFIX
		make 
		make install
		popd
		touch "$BASEDIR/${pkg_dirs[gdb]}/.installed"
	fi
}

package() {
	local FINALNAME="boff-mingw$NBITS-arm-gcc"
	find $PREFIX -iname '*.exe' -exec "$HOST-strip" {} \;
	cp "/usr/$HOST/bin/libgmp-10.dll" $PREFIX/bin
	cp "/usr/$HOST/bin/libmpfr-6.dll" $PREFIX/bin
	cp "/usr/$HOST/bin/libmpc-3.dll" $PREFIX/bin
	cp "/usr/$HOST/bin/libwinpthread-1.dll" $PREFIX/bin
	cp "/usr/$HOST/bin/libstdc++-6.dll" $PREFIX/bin
	if [ -f "/usr/$HOST/bin/libgcc_s_seh-1.dll" ]; then
		cp "/usr/$HOST/bin/libgcc_s_seh-1.dll" $PREFIX/bin
	fi
	if [ -f "/usr/$HOST/bin/libgcc_s_sjlj-1.dll" ]; then
		cp "/usr/$HOST/bin/libgcc_s_sjlj-1.dll" $PREFIX/bin
	fi

	mkdir -p $PREFIX/lib/bfd-plugins
	cp $PREFIX/libexec/gcc/$TARGET/$GCC_VER/liblto_plugin-0.dll $PREFIX/lib/bfd-plugins/
	7z a "$BASENAME.zip" $PREFIX
}



#Build
mkdir -p $BASEDIR $DLDIR
download
unpackall
build
package





