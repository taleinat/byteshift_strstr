#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "byteshift_strstr.h"
#include "make_big_endian.h"


/**
 * Finds the first occurrence of the sub-string needle in the string haystack.
 * Returns NULL if needle was not found.
 */
char *byteshift_strstr(const char *haystack, const char *needle)
{
    if (!*needle) // Empty needle.
        return (char *) haystack;

    const char    needle_first  = *needle;

    // Runs strchr() on the first section of the haystack as it has a lower
    // algorithmic complexity for discarding the first non-matching characters.
    haystack = strchr(haystack, needle_first);
    if (!haystack) // First character of needle is not in the haystack.
        return NULL;

    // First characters of haystack and needle are the same now. Both are
    // guaranteed to be at least one character long.
    // Now computes the sum of the first needle_len characters of haystack
    // minus the sum of characters values of needle.

    const char   *i_haystack    = haystack + 1
             ,   *i_needle      = needle   + 1;
    bool          identical     = true;

    while (*i_haystack && *i_needle) {
        identical &= *i_haystack++ == *i_needle++;
    }

    // i_haystack now references the (needle_len + 1)-th character.

    if (*i_needle) // haystack is smaller than needle.
        return NULL;
    else if (identical)
        return (char *) haystack;

    size_t        needle_len    = i_needle - needle;

    // Note: needle_len > 1, because we checked that it isn't zero, and if it
    //       is 1 then identical must be true because the first strchr() ensured
    //       that the first characters are identical

    const char   *sub_start = haystack;
    int compare_len;
    unsigned long first_needle_chars;
    unsigned long first_haystack_chars;
    unsigned long mask;

#ifdef MAKE_ULONG_BIGENDIAN
    first_needle_chars = MAKE_ULONG_BIGENDIAN(*(((unsigned long *)needle)));
    first_haystack_chars = MAKE_ULONG_BIGENDIAN(*(((unsigned long *)haystack)));
#else
    const char   *needle_end    = needle + LONG_INT_N_BYTES;
    size_t        min_cmp_len   = (needle_len < LONG_INT_N_BYTES) ? needle_len : LONG_INT_N_BYTES
    i_needle -= min_cmp_len;
    i_haystack -= min_cmp_len;
    first_needle_chars = 0;
    first_haystack_chars = 0;
    while (i_needle != needle_end) {
        first_needle_chars <<= 8;
        first_needle_chars ^= *i_needle++;
        first_haystack_chars <<= 8;
        first_haystack_chars ^= *i_haystack++;
    }
#endif

    // At this point:
    // * needle is at least two characters long
    // * haystack is at least needle_len characters long (also at least two)
    // * the first characters of needle and haystack are identical

    if (needle_len > LONG_INT_N_BYTES + 1)
    {
        /* we will call memcmp() only once we know that the LONG_INT_N_BYTES
           last chars are equal, so it will be enough to compare all but the
           last LONG_INT_N_BYTES characters */
        compare_len = needle_len - LONG_INT_N_BYTES;

        /* iterate through the remainder of haystack while checking for identity
           of the last LONG_INT_N_BYTES, and checking the rest with memcmp() */
        while (*i_haystack)
        {
            first_haystack_chars <<= 8;
            first_haystack_chars ^= *(unsigned char *)i_haystack++;

            sub_start++;
            if (   first_haystack_chars == first_needle_chars
                && memcmp(sub_start, needle, compare_len) == 0)
            {
                return (void *) sub_start;
            }
        }
    }
    else if (needle_len == LONG_INT_N_BYTES + 1)
    {
        /* iterate through the remainder of haystack while checking for identity
           of the last LONG_INT_N_BYTES as well as the single additional
           character, which is the first one */
        while (*i_haystack)
        {
            first_haystack_chars <<= 8;
            first_haystack_chars ^= *(unsigned char *)i_haystack++;

            sub_start++;
            if (   first_haystack_chars == first_needle_chars
                && *sub_start == needle_first)
            {
                return (void *) sub_start;
            }
        }
    }
    else if (needle_len == LONG_INT_N_BYTES)
    {
        /* iterate through the remainder of haystack while checking for identity
           of the last LONG_INT_N_BYTES characters, which should exactly match
           the entire needle */
        while (*i_haystack)
        {
            first_haystack_chars <<= 8;
            first_haystack_chars ^= *(unsigned char *)i_haystack++;

            if (first_haystack_chars == first_needle_chars)
            {
                return (void *) (i_haystack - needle_len);
            }
        }
    }
    else /* needle_len < LONG_INT_N_BYTES */
    {
        mask = (((unsigned long) 1) << (needle_len * 8)) - 1;
        first_needle_chars &= mask;

        /* iterate through the remainder of haystack, updating the sums' difference
           and checking for identity whenever the difference is zero */
        while (*i_haystack)
        {
            first_haystack_chars <<= 8;
            first_haystack_chars ^= *(unsigned char *)i_haystack++;
            first_haystack_chars &= mask;

            /* if sums_diff == 0, we know that the sums are equal, so it is enough
               to compare all but the last characters */
            if (first_haystack_chars == first_needle_chars)
            {
                return (void *) (i_haystack - needle_len);
            }
        }
    }

    return NULL;
}
