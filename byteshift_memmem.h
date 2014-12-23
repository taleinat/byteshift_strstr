#ifndef WORDLEN_MEMMEM_H
#define WORDLEN_MEMMEM_H

#include <stddef.h>

void *byteshift_memmem(const void *haystack, size_t haystack_len,
                       const void *needle, size_t needle_len);

#endif /* WORDLEN_MEMMEM_H */
