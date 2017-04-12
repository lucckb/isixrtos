#!/bin/sh
# Generate clang complete using WAF script tool
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
CC_ARGS="$DIR/cc_args.py"
CROSS_COMPILE="arm-none-eabi-"
export CC="python2 $CC_ARGS ${CROSS_COMPILE}gcc"
export CXX="python2 $CC_ARGS ${CROSS_COMPILE}g++"
waf "$@"

