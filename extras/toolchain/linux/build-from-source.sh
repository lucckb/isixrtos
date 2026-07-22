#!/usr/bin/env bash
#
# build-from-source.sh - Build arm-none-eabi toolchain on Linux from upstream sources.
#
# Mirrors the macOS (extras/toolchain/mac) and Windows (extras/toolchain/win) flows:
# binutils -> gcc (C only) -> newlib -> gcc (C/C++) -> gdb.
#
# Usage:
#   PREFIX=/opt/arm-none-eabi-gcc ./extras/toolchain/linux/build-from-source.sh
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=linuxbuild-common.env
source "${SCRIPT_DIR}/linuxbuild-common.env"

export PATH="${PREFIX}/bin:${PATH}"

gcc_common_configure_flags() {
	cat <<EOF
--target=${TARGET}
--prefix=${PREFIX}
--build=${CHOST}
--host=${CHOST}
--with-gnu-as
--with-gnu-ld
--with-newlib
--disable-nls
--disable-shared
--disable-threads
--disable-tls
--disable-libffi
--disable-libgomp
--disable-libmudflap
--disable-libquadmath
--disable-libssp
--disable-libstdcxx-pch
--disable-decimal-float
--with-multilib-list=rmprofile
EOF
}

prepare_dirs() {
	mkdir -p "${BASEDIR}" "${DLDIR}" "${PREFIX}"
}

download() {
	local index url bn
	for index in binutils gcc newlib gdb; do
		url="${dl_urls[$index]}"
		bn="$(basename "${url}")"
		if [[ ! -f "${DLDIR}/${bn}" ]]; then
			echo "Downloading ${bn} ..."
			curl -fsSL "${url}" -o "${DLDIR}/${bn}"
		fi
	done
}

unpackall() {
	local index item diri
	for index in binutils gcc newlib gdb; do
		item="$(basename "${dl_urls[$index]}")"
		diri="${pkg_dirs[$index]}"
		if [[ ! -d "${BASEDIR}/${diri}" ]]; then
			echo "Unpacking ${item} ..."
			tar -xf "${DLDIR}/${item}" -C "${BASEDIR}"
		fi
	done
}

build_binutils() {
	local compname="${BASEDIR}/${pkg_dirs[binutils]}"
	if [[ -f "${compname}/.installed" ]]; then
		return
	fi

	echo "Building binutils ..."
	pushd "${compname}" >/dev/null
	./configure \
		--target="${TARGET}" \
		--prefix="${PREFIX}" \
		--build="${CHOST}" \
		--host="${CHOST}" \
		--disable-nls \
		--disable-werror \
		--disable-sim \
		--disable-gdb \
		--enable-interwork \
		--enable-plugins
	make -j"${BUILD_JOBS}"
	make install
	popd >/dev/null
	touch "${compname}/.installed"
}

build_gcc_base() {
	local compname="${BASEDIR}/${pkg_dirs[gcc]}"
	if [[ -f "${compname}/.installed-base" ]]; then
		return
	fi

	echo "Building GCC (base, C only) ..."
	rm -rf "${BASEDIR}/base-gcc"
	mkdir -p "${BASEDIR}/base-gcc"
	pushd "${BASEDIR}/base-gcc" >/dev/null
	"${compname}/configure" \
		$(gcc_common_configure_flags) \
		--enable-languages=c \
		--without-headers
	make -j"${BUILD_JOBS}" all-gcc
	make install-gcc
	popd >/dev/null
	touch "${compname}/.installed-base"
}

build_newlib() {
	local compname="${BASEDIR}/${pkg_dirs[newlib]}"
	if [[ -f "${compname}/.installed" ]]; then
		return
	fi

	echo "Building newlib ..."
	pushd "${compname}" >/dev/null
	./configure \
		--target="${TARGET}" \
		--prefix="${PREFIX}" \
		--build="${CHOST}" \
		--host="${CHOST}" \
		--disable-newlib-supplied-syscalls \
		--enable-newlib-reent-check-verify \
		--enable-newlib-reent-small \
		--enable-newlib-retargetable-locking \
		--disable-newlib-fvwrite-in-streamio \
		--disable-newlib-fseek-optimization \
		--disable-newlib-wide-orient \
		--enable-newlib-nano-malloc \
		--disable-newlib-unbuf-stream-opt \
		--enable-lite-exit \
		--enable-newlib-global-atexit \
		--enable-newlib-nano-formatted-io \
		--disable-nls
	make -j"${BUILD_JOBS}"
	make install
	popd >/dev/null
	touch "${compname}/.installed"
}

build_gcc_final() {
	local compname="${BASEDIR}/${pkg_dirs[gcc]}"
	if [[ -f "${compname}/.installed-full" ]]; then
		return
	fi

	echo "Building GCC (final, C/C++) ..."
	rm -rf "${BASEDIR}/build-gcc"
	mkdir -p "${BASEDIR}/build-gcc"
	pushd "${BASEDIR}/build-gcc" >/dev/null
	"../${pkg_dirs[gcc]}/configure" \
		$(gcc_common_configure_flags) \
		--enable-languages=c,c++ \
		--enable-plugins \
		--enable-lto \
		--with-headers=yes
	make -j"${BUILD_JOBS}" INHIBIT_LIBC_CFLAGS="-DUSE_TM_CLONE_REGISTRY=0"
	make install
	popd >/dev/null
	touch "${compname}/.installed-full"
}

build_gdb() {
	local compname="${BASEDIR}/${pkg_dirs[gdb]}"
	if [[ -f "${compname}/.installed" ]]; then
		return
	fi

	echo "Building gdb ..."
	rm -rf "${BASEDIR}/build-gdb"
	mkdir -p "${BASEDIR}/build-gdb"
	pushd "${BASEDIR}/build-gdb" >/dev/null
	"${compname}/configure" \
		--target="${TARGET}" \
		--prefix="${PREFIX}" \
		--build="${CHOST}" \
		--host="${CHOST}" \
		--disable-nls \
		--enable-interwork \
		--enable-multilib \
		--with-float=soft \
		--disable-shared \
		--disable-libgomp \
		--disable-libmudflap \
		--disable-libssp \
		--disable-werror \
		--enable-lto \
		--without-auto-load-safe-path \
		--with-guile=no
	make -j"${BUILD_JOBS}"
	make install
	popd >/dev/null
	touch "${compname}/.installed"
}

main() {
	if [[ -x "${PREFIX}/bin/${TARGET}-gcc" ]]; then
		echo "Toolchain already installed at ${PREFIX}"
		"${PREFIX}/bin/${TARGET}-gcc" --version | sed -n '1p'
		"${PREFIX}/bin/${TARGET}-gcc" -print-file-name=include/sys/types.h
		exit 0
	fi

	command -v curl >/dev/null || { echo "build-from-source.sh: curl is required" >&2; exit 1; }
	command -v gcc >/dev/null || { echo "build-from-source.sh: host gcc is required" >&2; exit 1; }

	prepare_dirs
	download
	unpackall
	build_binutils
	build_gcc_base
	build_newlib
	build_gcc_final
	build_gdb

	echo "Installed:"
	"${PREFIX}/bin/${TARGET}-gcc" --version | sed -n '1p'
	"${PREFIX}/bin/${TARGET}-g++" --version | sed -n '1p'
	"${PREFIX}/bin/${TARGET}-gcc" -print-file-name=include/sys/types.h
}

main "$@"
