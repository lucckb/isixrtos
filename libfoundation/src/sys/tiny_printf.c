/*
 * tiny_printf.c
 *
 * Minimal printf without float
 * Author: Lucjan Bryndza
 */

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

#include "foundation/sys/tiny_printf.h"
#include "foundation/sys/tiny_vaprintf.h"

/* Main handlers */
static int  (*fn_putc)(int, void* ) = NULL;
static void *fn_putc_arg            = NULL;
static void (*fn_lock)(void)        = NULL;
static void (*fn_unlock)(void)      = NULL;

/* Send single char via API*/
static void printchar(char **str, size_t len, size_t *pc, int c)
{
    if (str) {
        /* snprintf mode - check buffer len*/
        if (*pc + 1 < len) {
            **str = (char)c;
            (*str)++;
        }
    } else if (fn_putc) {
        /* printf mode – putchar */
        fn_putc(c, fn_putc_arg);
    }
    (*pc)++;
}

int tiny_putchar(int ch)
{
    if (fn_putc)
        return fn_putc(ch, fn_putc_arg);
    return ch;
}

int tiny_puts(const char *str)
{
    int count = 0;
    while (*str) {
        tiny_putchar(*str++);
        count++;
    }
    tiny_putchar('\r');
    tiny_putchar('\n');
    return count + 2;
}

#define PAD_RIGHT 1
#define PAD_ZERO  2

static int prints(char **out, size_t len, size_t *pc,
                  const char *string, int width, int pad)
{
    size_t pc_start = *pc;
    char padchar = ' ';

    if (width > 0) {
        int slen = 0;
        const char *ptr;
        for (ptr = string; *ptr; ++ptr) ++slen;
        if (slen >= width) {
            width = 0;
        } else {
            width -= slen;
        }
        if (pad & PAD_ZERO)
            padchar = '0';
    }

    if (!(pad & PAD_RIGHT)) {
        for (; width > 0; --width) {
            printchar(out, len, pc, padchar);
        }
    }
    for (; *string; ++string) {
        printchar(out, len, pc, *string);
    }
    for (; width > 0; --width) {
        printchar(out, len, pc, padchar);
    }

    return (int)(*pc - pc_start);
}

/* Buffer for 32 and 64 bit items */
#define PRINT_BUF_LEN 32

static int printi(char **out, size_t len, size_t *pc,
                  long long i, int base, int sign,
                  int width, int pad, int letbase)
{
    char print_buf[PRINT_BUF_LEN];
    char *s;
    int t, neg = 0;
    unsigned long long u;

    if (i == 0) {
        print_buf[0] = '0';
        print_buf[1] = '\0';
        return prints(out, len, pc, print_buf, width, pad);
    }

    if (sign && base == 10 && i < 0) {
        neg = 1;
        u = (unsigned long long)(-i);
    } else {
        u = (unsigned long long)i;
    }

    s = print_buf + PRINT_BUF_LEN - 1;
    *s = '\0';

    while (u) {
        t = (int)(u % base);
        if (t >= 10)
            t += letbase - '0' - 10;
        *--s = (char)(t + '0');
        u /= base;
    }

    if (neg) {
        if (width && (pad & PAD_ZERO)) {
            printchar(out, len, pc, '-');
            --width;
        } else {
            *--s = '-';
        }
    }

    return prints(out, len, pc, s, width, pad);
}

/* -------------------------------------------------------
 *  tiny_vaprintf – main parser (without va_end!)
 * ----------------------------------------------------- */
int tiny_vaprintf(char **out, size_t len,
                  const char *format, va_list args)
{
    enum { m_hh, m_h, m_n, m_l, m_ll };
    int width, pad;
    size_t pc = 0;
    char scr[2];

    while (*format) {
        if (*format != '%') {
            /* Zwykły znak */
            printchar(out, len, &pc, *format++);
            continue;
        }

        /* Format specifier start */
        ++format;
        width = 0;
        pad = 0;
        int modf = m_n;

        /* "%%" -> single '%' */
        if (*format == '%') {
            printchar(out, len, &pc, *format++);
            continue;
        }

        /* flags */
        if (*format == '-') {
            pad = PAD_RIGHT;
            ++format;
        }
        while (*format == '0') {
            pad |= PAD_ZERO;
            ++format;
        }

        /* width */
        while (*format >= '0' && *format <= '9') {
            width = width * 10 + (*format - '0');
            ++format;
        }

        /* length mod */
        if (*format == 'h') {
            ++format;
            if (*format == 'h') {
                modf = m_hh;
                ++format;
            } else {
                modf = m_h;
            }
        } else if (*format == 'l') {
            ++format;
            if (*format == 'l') {
                modf = m_ll;
                ++format;
            } else {
                modf = m_l;
            }
        } else if (*format == 'z') {
            ++format;
            modf = m_l;   /* size_t for 32-bit */
        }

        char spec = *format ? *format++ : '\0';

        switch (spec) {
        case 's': {
            const char *s = va_arg(args, const char*);
            if (!s) s = "(null)";
            prints(out, len, &pc, s, width, pad);
            break;
        }

        case 'd':
        case 'i': {
            long long v;
            switch (modf) {
            case m_hh: v = (signed char)va_arg(args, int);       break;
            case m_h:  v = (signed short)va_arg(args, int);      break;
            case m_l:  v = va_arg(args, long);                   break;
            case m_ll: v = va_arg(args, long long);              break;
            default:   v = va_arg(args, int);                    break;
            }
            printi(out, len, &pc, v, 10, 1, width, pad, 'a');
            break;
        }

        case 'u':
        case 'x':
        case 'X': {
            unsigned long long v;
            switch (modf) {
            case m_hh: v = (unsigned char)va_arg(args, unsigned int);   break;
            case m_h:  v = (unsigned short)va_arg(args, unsigned int);  break;
            case m_l:  v = va_arg(args, unsigned long);                 break;
            case m_ll: v = va_arg(args, unsigned long long);            break;
            default:   v = va_arg(args, unsigned int);                  break;
            }
            int base    = (spec == 'u') ? 10 : 16;
            int letbase = (spec == 'x') ? 'a' : 'A';
            printi(out, len, &pc, (long long)v, base, 0, width, pad, letbase);
            break;
        }

        case 'p': {
            uintptr_t v = (uintptr_t)va_arg(args, void*);
            printi(out, len, &pc,
                   (long long)v, 16, 0,
                   (int)(sizeof(void*) * 2),
                   PAD_ZERO, 'A');
            break;
        }

        case 'c': {
            scr[0] = (char)va_arg(args, int);
            scr[1] = '\0';
            prints(out, len, &pc, scr, width, pad);
            break;
        }

        case '\0':
            /* Format end */
            return (int)pc;

        default:
            /* unknown specifier – print as it is */
            printchar(out, len, &pc, '%');
            printchar(out, len, &pc, spec);
            break;
        }
    }

    if (out && *out && len > 0) {
        **out = '\0';
    }

    return (int)pc;
}

/* -------------------------------------------------------
 *  tiny_printf / tiny_snprintf – tiny_vaprintf wrapper
 * ----------------------------------------------------- */

int tiny_printf(const char *format, ...)
{
    int result;
    va_list args;
    va_start(args, format);
    if (fn_lock)   fn_lock();
    result = tiny_vaprintf(NULL, 0, format, args);
    if (fn_unlock) fn_unlock();
    va_end(args);
    return result;
}

int tiny_snprintf(char *out, unsigned long max_len, const char *format, ...)
{
    int result;
    va_list args;
    char *p = out;
    va_start(args, format);
    result = tiny_vaprintf(&p, max_len, format, args);
    va_end(args);
    return result;
}

/* -------------------------------------------------------
 *  Handlers registration
 * ----------------------------------------------------- */

void register_printf_putc_handler_syslock(int (*fputc)(int,void*), void *arg,
                                          void (*lock)(void), void (*unlock)(void))
{
    fn_putc     = fputc;
    fn_putc_arg = arg;
    fn_lock     = lock;
    fn_unlock   = unlock;
}

