/*
 * =====================================================================================
 *
 *       Filename:  config.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06.10.2015 07:32:00
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once
#ifdef _HAVE_MBEDTLS_CONFIG_H
#include <mbedtls_config.h>
#include "isixcommon_config.h"
#include "check_config.h"

#else
/* By default only minimal config */
/* mbed TLS feature support */
#define MBEDTLS_KEY_EXCHANGE_PSK_ENABLED
#define MBEDTLS_SSL_PROTO_TLS1_2

//#define MBEDTLS_DEBUG_C
//#define MBEDTLS_SSL_DEBUG_ALL

/* mbed TLS modules */
#define MBEDTLS_AES_C
#define MBEDTLS_CCM_C
#define MBEDTLS_CIPHER_C
#define MBEDTLS_CTR_DRBG_C
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_MD_C
#define MBEDTLS_NET_C
#define MBEDTLS_SHA256_C
#define MBEDTLS_SSL_CLI_C
#define MBEDTLS_SSL_SRV_C
#define MBEDTLS_SSL_TLS_C
/* Save RAM at the expense of ROM */
#define MBEDTLS_AES_ROM_TABLES
/* Save some RAM by adjusting to your exact needs */
#define MBEDTLS_PSK_MAX_LEN    16 /* 128-bits keys are generally enough */
/*
 * Use only CCM_8 ciphersuites, and
 * save ROM and a few bytes of RAM by specifying our own ciphersuite list
 */
#define MBEDTLS_SSL_CIPHERSUITES                        \
        MBEDTLS_TLS_PSK_WITH_AES_256_CCM_8,             \
        MBEDTLS_TLS_PSK_WITH_AES_128_CCM_8

#define MBEDTLS_SSL_MAX_CONTENT_LEN             512
#include "isixcommon_config.h"
#include "check_config.h"

#endif
