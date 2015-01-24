#include <limits.h>
#include <string.h>
#include "byteshift_memmem.h"
#include "make_big_endian.h"


void *byteshift_memmem(const void *haystack, size_t haystack_len,
                       const void *needle, size_t needle_len)
{
    const unsigned char* needle_ptr;
    const unsigned char* haystack_ptr;
    int sums_diff;
    size_t compare_len;
    unsigned long last_needle_chars;
    unsigned long last_haystack_chars;
    unsigned int i;

    switch (needle_len) {
        case (0):
            /* empty needle */
            return (void *) haystack;
            break;
        case (1):
            /* special case for single-character needles */
            return memchr(haystack, *((unsigned char*) needle), haystack_len);
            break;
    }

    /* start searching through haystack only from the first occurence of the
       first character of needle */
    haystack_ptr = (const unsigned char *) memchr(haystack, *((const unsigned char*) needle), haystack_len);
    if (!haystack_ptr) {
        /* the first character of needle isn't in haystack */
        return NULL;
    }
    haystack_len -= (haystack_ptr - (const unsigned char*)haystack);
    if (haystack_len < needle_len) {
        /* the remaining haystack is smaller than needle */
        return NULL;
    }
    haystack = (void *)haystack_ptr;

    if (needle_len > LONG_INT_N_BYTES + 1)
    {
        needle_ptr = (const unsigned char *)needle;
        sums_diff = 0;
#ifndef MAKE_ULONG_BIGENDIAN
        for (i = needle_len - LONG_INT_N_BYTES; i > 0; --i) {
            sums_diff -= *needle_ptr++;
            sums_diff += *haystack_ptr++;
        }

        last_needle_chars = 0;
        last_haystack_chars = 0;
        for (i = LONG_INT_N_BYTES; i > 0; --i) {
            last_needle_chars <<= 8;
            last_needle_chars ^= *needle_ptr;
            last_haystack_chars <<= 8;
            last_haystack_chars ^= *haystack_ptr;
            sums_diff -= *needle_ptr++;
            sums_diff += *haystack_ptr++;
        }
#else
        for (i = needle_len; i > 0; --i) {
            sums_diff -= *needle_ptr++;
            sums_diff += *haystack_ptr++;
        }

        last_needle_chars = MAKE_ULONG_BIGENDIAN(*(((unsigned long *)needle_ptr) - 1));
        last_haystack_chars = MAKE_ULONG_BIGENDIAN(*(((unsigned long *)haystack_ptr) - 1));

#endif /* MAKE_ULONG_BIGENDIAN */

        /* we will call memcmp() only once we know that the sums are equal and
           that LONG_INT_N_BYTES last chars are equal, so it will be enough to
           compare all but the last LONG_INT_N_BYTES + 1 characters */
        compare_len = needle_len - (LONG_INT_N_BYTES + 1);

        /* At this point:
           * needle is at least two characters long
           * haystack is at least needle_len characters long (also at least two)
           * the first characters of needle and haystack are identical
        */

        if (   sums_diff == 0
            && last_haystack_chars == last_needle_chars
            && memcmp(haystack, needle, compare_len) == 0)
        {
            return (void *) haystack;
        }

        /* iterate through the remainder of haystack, updating the sums' difference
           and checking for identity whenever the difference is zero */
        for (i = haystack_len - needle_len; i > 0; --i)
        {
            last_haystack_chars <<= 8;
            last_haystack_chars ^= *haystack_ptr;
            sums_diff -= *(const unsigned char*)haystack++;
            sums_diff += *haystack_ptr++;

            /* if sums_diff == 0, we know that the sums are equal, so it is enough
               to compare all but the last characters */
            if (   sums_diff == 0
                && last_haystack_chars == last_needle_chars
                && memcmp(haystack, needle, compare_len) == 0)
            {
                return (void *) haystack;
            }
        }
    }
    else if (needle_len < LONG_INT_N_BYTES)
    {
        needle_ptr = (const unsigned char *)needle;
        sums_diff = 0;
        for (i = needle_len; i > 0; --i) {
            sums_diff -= *needle_ptr++;
            sums_diff += *haystack_ptr++;
        }

        /* we will call memcmp() only once we know that the sums are equal, so
           it will be enough to compare all but the last characters */
        compare_len = needle_len - 1;

        /* At this point:
           * needle is at least two characters long
           * haystack is at least needle_len characters long (also at least two)
           * the first characters of needle and haystack are identical
        */
        if (   sums_diff == 0
            && memcmp(haystack, needle, compare_len) == 0)
        {
            return (void *) haystack;
        }

        /* iterate through the remainder of haystack, updating the sums' difference
           and checking for identity whenever the difference is zero */
        for (i = haystack_len - needle_len; i > 0; --i)
        {
            sums_diff -= *(const unsigned char *)haystack++;
            sums_diff += *haystack_ptr++;

            /* if sums_diff == 0, we know that the sums are equal, so it is enough
               to compare all but the last characters */
            if (   sums_diff == 0
                && memcmp(haystack, needle, compare_len) == 0)
            {
                return (void *) haystack;
            }
        }
    }
    else if (needle_len == LONG_INT_N_BYTES)
    {
#ifndef MAKE_ULONG_BIGENDIAN
        needle_ptr = (const unsigned char *)needle;
        last_needle_chars = 0;
        last_haystack_chars = 0;
        for (i = needle_len; i > 0; --i) {
            last_needle_chars <<= 8;
            last_needle_chars ^= *needle_ptr++;
            last_haystack_chars <<= 8;
            last_haystack_chars ^= *haystack_ptr++;
        }
#else
        last_needle_chars = MAKE_ULONG_BIGENDIAN(*(unsigned long *)needle);
        last_haystack_chars = MAKE_ULONG_BIGENDIAN(*(unsigned long *)haystack);
        haystack_ptr += LONG_INT_N_BYTES;
#endif /* MAKE_ULONG_BIGENDIAN */

        if (last_haystack_chars == last_needle_chars)
        {
            return (void *) haystack;
        }

        /* iterate through the remainder of haystack, updating the last char
           data and checking for equality */
        for (i = haystack_len - needle_len; i > 0; --i)
        {
            last_haystack_chars <<= 8;
            last_haystack_chars ^= *haystack_ptr++;
            if (last_haystack_chars == last_needle_chars)
            {
                return (void *) (haystack_ptr - needle_len);
            }
        }
    }
    else /* needle_len == LONG_INT_N_BYTES + 1 */
    {
#ifndef MAKE_ULONG_BIGENDIAN
        needle_ptr = (const unsigned char *)needle;
        last_needle_chars = 0;
        last_haystack_chars = 0;
       for (i = LONG_INT_N_BYTES; i > 0; --i) {
            last_needle_chars <<= 8;
            last_needle_chars ^= *needle_ptr++;
            last_haystack_chars <<= 8;
            last_haystack_chars ^= *haystack_ptr++;
        }
#else
        last_needle_chars = MAKE_ULONG_BIGENDIAN(*(unsigned long *)needle);
        last_haystack_chars = MAKE_ULONG_BIGENDIAN(*(unsigned long *)haystack);
        haystack_ptr += LONG_INT_N_BYTES;
#endif /* MAKE_ULONG_BIGENDIAN */

        unsigned char last_needle_char = *(((const unsigned char *)needle) + LONG_INT_N_BYTES);

        if (   last_haystack_chars == last_needle_chars
            && *haystack_ptr == last_needle_char)
        {
            return (void *) haystack;
        }

        /* iterate through the remainder of haystack, updating the last char
           data and checking for equality */
        for (i = haystack_len - needle_len; i > 0; --i)
        {
            last_haystack_chars <<= 8;
            last_haystack_chars ^= *haystack_ptr++;
            if (   last_haystack_chars == last_needle_chars
                && *haystack_ptr == last_needle_char)
            {
                return (void *) (haystack_ptr - (needle_len - 1));
            }
        }
    }

    return NULL;
}
