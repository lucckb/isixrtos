#!/bin/sh
# Generate clang complete using WAF script tool
PYCC=$(command -v python2)
if [[ ! -z $PYCC ]]; then
	PYCC=python2
else
	PYCC=
fi
if [[ -z $PYCC ]]; then
	PYCC=python
fi

if [[ -z $PYCC ]]; then
	echo "Unable to find python2 interpreter"
	exit -1
fi
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
CC_ARGS="$DIR/cc_args.py"
CROSS_COMPILE="arm-none-eabi-"
export CC="$PYCC $CC_ARGS ${CROSS_COMPILE}gcc"
export CXX="$PYCC $CC_ARGS ${CROSS_COMPILE}g++"
waf "$@"

