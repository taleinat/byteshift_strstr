#include <limits.h>
#include <string.h>
#include "byteshift_memmem.h"
#include "make_big_endian.h"


void *byteshift_memmem(const void *haystack, size_t haystack_len,
                       const void *needle, size_t needle_len)
{
    const void* needle_ptr;
    const void* haystack_ptr;
    int sums_diff;
    int compare_len;
    unsigned long first_needle_chars = 0;
    unsigned long first_haystack_chars = 0;

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
    haystack_ptr = memchr(haystack, *((unsigned char*) needle), haystack_len);
    if (!haystack_ptr) {
        /* the first character of needle isn't in haystack */
        return NULL;
    }
    haystack_len -= (haystack_ptr - haystack);
    if (haystack_len < needle_len) {
        /* the remaining haystack is smaller than needle */
        return NULL;
    }
    haystack = haystack_ptr;


    if (needle_len > LONG_INT_N_BYTES + 1)
    {
        needle_ptr = needle;
        sums_diff = 0;
#ifndef MAKE_ULONG_BIGENDIAN
        needle += LONG_INT_N_BYTES;
        while (needle_ptr != needle) {
            first_needle_chars <<= 8;
            first_needle_chars ^= *(unsigned char *)needle_ptr;
            first_haystack_chars <<= 8;
            first_haystack_chars ^= *(unsigned char *)haystack_ptr;
            sums_diff -= *((unsigned char *) needle_ptr++);
            sums_diff += *((unsigned char *) haystack_ptr++);
        }

        needle += needle_len - LONG_INT_N_BYTES;
        while (needle_ptr != needle) {
            sums_diff -= *((unsigned char *) needle_ptr++);
            sums_diff += *((unsigned char *) haystack_ptr++);
        }
#else
        first_needle_chars = MAKE_ULONG_BIGENDIAN(*(((unsigned long *)needle)));
        first_haystack_chars = MAKE_ULONG_BIGENDIAN(*(((unsigned long *)haystack)));

        needle += needle_len;
        while (needle_ptr != needle) {
            sums_diff -= *((unsigned char *) needle_ptr++);
            sums_diff += *((unsigned char *) haystack_ptr++);
        }
#endif /* MAKE_ULONG_BIGENDIAN */
        needle -= needle_len;

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
            && first_haystack_chars == first_needle_chars
            && memcmp(haystack, needle, compare_len) == 0)
        {
            return (void *) haystack;
        }

        /* iterate through the remainder of haystack, updating the sums' difference
           and checking for identity whenever the difference is zero */
        needle_ptr = haystack + haystack_len;
        while (haystack_ptr != needle_ptr)
        {
            first_haystack_chars <<= 8;
            first_haystack_chars ^= *(unsigned char *)haystack_ptr;
            sums_diff -= *((unsigned char *) haystack++);
            sums_diff += *((unsigned char *) haystack_ptr++);

            /* if sums_diff == 0, we know that the sums are equal, so it is enough
               to compare all but the last characters */
            if (   sums_diff == 0
                && first_haystack_chars == first_needle_chars
                && memcmp(haystack, needle, compare_len) == 0)
            {
                return (void *) haystack;
            }
        }
    }
    else if (needle_len < LONG_INT_N_BYTES)
    {
        needle_ptr = needle;
        needle += needle_len;
        sums_diff = 0;
        while (needle_ptr != needle) {
            sums_diff -= *((unsigned char *) needle_ptr++);
            sums_diff += *((unsigned char *) haystack_ptr++);
        }
        needle -= needle_len;

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
        needle_ptr = haystack + haystack_len;
        while (haystack_ptr != needle_ptr)
        {
            sums_diff -= *((unsigned char *) haystack++);
            sums_diff += *((unsigned char *) haystack_ptr++);

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
        needle_ptr = needle;
        needle += needle_len;
        while (needle_ptr != needle) {
            first_needle_chars <<= 8;
            first_needle_chars ^= *(unsigned char *)needle_ptr++;
            first_haystack_chars <<= 8;
            first_haystack_chars ^= *(unsigned char *)haystack_ptr++;
        }
        needle -= needle_len;
#else
        first_needle_chars = MAKE_ULONG_BIGENDIAN(*(unsigned long *)needle);
        first_haystack_chars = MAKE_ULONG_BIGENDIAN(*(unsigned long *)haystack);
        haystack_ptr += LONG_INT_N_BYTES;
#endif /* MAKE_ULONG_BIGENDIAN */

        if (first_haystack_chars == first_needle_chars)
        {
            return (void *) haystack;
        }

        /* iterate through the remainder of haystack, updating the last char
           data and checking for equality */
        needle_ptr = haystack + haystack_len;
        while (haystack_ptr != needle_ptr)
        {
            first_haystack_chars <<= 8;
            first_haystack_chars ^= *(unsigned char *)haystack_ptr++;
            if (first_haystack_chars == first_needle_chars)
            {
                return (void *) (haystack_ptr - needle_len);
            }
        }
    }
    else /* needle_len == LONG_INT_N_BYTES + 1 */
    {
        needle_ptr = needle + 1;
        ++haystack_ptr; /* equivalent to: haystack_ptr = haystack + 1 */
#ifndef MAKE_ULONG_BIGENDIAN
        needle += needle_len;
        while (needle_ptr != needle) {
            first_needle_chars <<= 8;
            first_needle_chars ^= *(unsigned char *)needle_ptr++;
            first_haystack_chars <<= 8;
            first_haystack_chars ^= *(unsigned char *)haystack_ptr++;
        }
        needle -= needle_len;
#else
        first_needle_chars = MAKE_ULONG_BIGENDIAN(*(unsigned long *)needle_ptr);
        first_haystack_chars = MAKE_ULONG_BIGENDIAN(*(unsigned long *)haystack_ptr);
        haystack_ptr += LONG_INT_N_BYTES;
#endif /* MAKE_ULONG_BIGENDIAN */
        if (   first_haystack_chars == first_needle_chars
            && *(unsigned char *)haystack == *(unsigned char *)needle)
        {
            return (void *) haystack;
        }

        /* iterate through the remainder of haystack, updating the last char
           data and checking for equality */
        needle_ptr = haystack + haystack_len;
        while (haystack_ptr != needle_ptr)
        {
            first_haystack_chars <<= 8;
            first_haystack_chars ^= *(unsigned char *)haystack_ptr++;

            ++haystack;
            if (   first_haystack_chars == first_needle_chars
                && *(unsigned char *)haystack == *(unsigned char *)needle)
            {
                return (void *) haystack;
            }
        }
    }

    return NULL;
}
