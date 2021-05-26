#!/usr/local/bin/bash -e
source macbuild-common.env

build() {
	#Binutils
	compname="$BASEDIR/${pkg_dirs[binutils]}"
	if [ ! -f "$compname/.compiled" ]; then
		echo "Compile binutils..."
		pushd $compname
		./configure \
			--target=$TARGET \
			--prefix=$PREFIX \
			--build=$CHOST \
			--host=$CHOST \
			--disable-nls \
			--disable-werror \
			--disable-sim \
			--disable-gdb \
			--enable-interwork \
			--enable-plugins 

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
mkdir -p $BASEDIR $DLDIR
prepare_dirs
download
unpackall
build

