/**
 * Define a MAKE_ULONG_BIGENDIAN(x) macro, as well as LONG_INT_N_BYTES.
 */
#ifndef MAKE_BIG_ENDIAN_H
#define MAKE_BIG_ENDIAN_H

/**
 * Include endian related definitions.
 */
#if defined(__APPLE__)
    /* See: https://gist.github.com/yinyin/2027912
       Or search for: OSSwapHostToBigInt64 */
    #include <libkern/OSByteOrder.h>
    #define htobe64(x) OSSwapHostToBigInt64(x)
    /* TODO: is this needed? causes redefinition warnings */
    #define htonl(x) OSSwapHostToBigInt32(x)
#elif defined(BSD)
    #if defined(__OpenBSD__)
        #include <sys/types.h>
    #else
        #include <sys/endian.h>
    #endif
#else
    #include <endian.h>
#endif

/**
 * Endian detection, partially based on: http://esr.ibiblio.org/?p=5095
 *
 * Define WORDS_BIGENDIAN if this big-endian or leave it undefined if
 * little-endian. If endianness can't be determined, define UNKNOWN_ENDIANNESS.
 */

/**
 *  __BIG_ENDIAN__ and __LITTLE_ENDIAN__ are defined in some gcc versions
 * only, probably depending on the architecture. Try to use endian.h if
 * the gcc way fails - endian.h also doesn not seem to be available on all
 * platforms.
 */
#if defined(__BIG_ENDIAN__)
    #define WORDS_BIGENDIAN 1
#elif defined(__LITTLE_ENDIAN__)
    #undef WORDS_BIGENDIAN
#elif defined(_WIN32)
    /* we assume all Windows platforms are little-endian */
    #undef WORDS_BIGENDIAN
#elif defined(__BYTE_ORDER) && defined(__BIG_ENDIAN) && defined(__LITTLE_ENDIAN)
    #if __BYTE_ORDER == __BIG_ENDIAN
        #define WORDS_BIGENDIAN 1
    #elif __BYTE_ORDER == __LITTLE_ENDIAN
        #undef WORDS_BIGENDIAN
    #else
        #define UNKNOWN_ENDIANNESS 1
    #endif /* __BYTE_ORDER */
#else
    #define UNKNOWN_ENDIANNESS 1
#endif

/**
 * Determine the size, in bytes, of a long integer.
 */

#if LONG_MAX == 2147483647
    #define LONG_INT_IS_4_BYTES 1
    #define LONG_INT_N_BYTES 4
#elif LONG_MAX == 9223372036854775807
    #define LONG_INT_IS_8_BYTES 1
    #define LONG_INT_N_BYTES 8
#else
    #define LONG_INT_IS_UNSUPPORTED_SIZE
    #define LONG_INT_N_BYTES sizeof(long)
#endif

/**
 * Finally, we can define the MAKE_BIGENDIAN(x) macro.
 */
#if !defined(UNKNOWN_ENDIANNESS) && defined(WORDS_BIGENDIAN)
    #define MAKE_ULONG_BIGENDIAN(x) (x)
#else
    #if defined(LONG_INT_IS_8_BYTES)
        #define MAKE_ULONG_BIGENDIAN(x) htobe64((x))
    #elif defined(LONG_INT_IS_4_BYTES)
        #define MAKE_ULONG_BIGENDIAN(x) htonl((x))
    #else
        #undef MAKE_ULONG_BIGENDIAN
    #endif
#endif /* !defined(UNKNOWN_ENDIANNESS) && defined(WORDS_BIGENDIAN) */


#undef UNKNOWN_ENDIANNESS
#undef WORDS_BIGENDIAN
#undef LONG_INT_IS_4_BYTES
#undef LONG_INT_IS_8_BYTES
#undef LONG_INT_IS_UNSUPPORTED_SIZE

#endif /* MAKE_BIG_ENDIAN_H */