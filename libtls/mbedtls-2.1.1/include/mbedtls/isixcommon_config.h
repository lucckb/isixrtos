/*
 * =====================================================================================
 *
 *       Filename:  isixcommon_config.h
 *
 *    Description:  Isix common configuration file
 *
 *        Version:  1.0
 *        Created:  06.10.2015 07:46:09
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

#include <foundation/tiny_printf.h>

/**
 * \def MBEDTLS_NO_PLATFORM_ENTROPY
 *
 * Do not use built-in platform entropy functions.
 * This is useful if your platform does not support
 * standards like the /dev/urandom or Windows CryptoAPI.
 *
 * Uncomment this macro to disable the built-in platform entropy functions.
 */
#define MBEDTLS_NO_PLATFORM_ENTROPY

/**
 * \def MBEDTLS_NET_C
 *
 * Enable the TCP/IP networking routines.
 *
 * Module:  library/net.c
 *
 * This module provides TCP/IP networking routines.
 */
#define MBEDTLS_NET_C

/**
 * \def MBEDTLS_PLATFORM_C
 *
 * Enable the platform abstraction layer that allows you to re-assign
 * functions like calloc(), free(), snprintf(), printf(), fprintf(), exit().
 *
 * Enabling MBEDTLS_PLATFORM_C enables to use of MBEDTLS_PLATFORM_XXX_ALT
 * or MBEDTLS_PLATFORM_XXX_MACRO directives, allowing the functions mentioned
 * above to be specified at runtime or compile time respectively.
 *
 * \note This abstraction layer must be enabled on Windows (including MSYS2)
 * as other module rely on it for a fixed snprintf implementation.
 *
 * Module:  library/platform.c
 * Caller:  Most other .c files
 *
 * This module enables abstraction of common (libc) functions.
 */
#define MBEDTLS_PLATFORM_C

/* To Use Function Macros MBEDTLS_PLATFORM_C must be enabled */
/* MBEDTLS_PLATFORM_XXX_MACRO and MBEDTLS_PLATFORM_XXX_ALT cannot both be defined */
//#define MBEDTLS_PLATFORM_CALLOC_MACRO        calloc /**< Default allocator macro to use, can be undefined */
//#define MBEDTLS_PLATFORM_FREE_MACRO            free /**< Default free macro to use, can be undefined */
#define MBEDTLS_PLATFORM_EXIT_MACRO            _unimplemented_exit /**< Default exit macro to use, can be undefined */
#define MBEDTLS_PLATFORM_FPRINTF_MACRO      _unimplemented_fprintf /**< Default fprintf macro to use, can be undefined */
#define MBEDTLS_PLATFORM_PRINTF_MACRO        tiny_printf /**< Default printf macro to use, can be undefined */
/* Note: your snprintf must correclty zero-terminate the buffer! */
#define MBEDTLS_PLATFORM_SNPRINTF_MACRO    snprintf /**< Default snprintf macro to use, can be undefined */
