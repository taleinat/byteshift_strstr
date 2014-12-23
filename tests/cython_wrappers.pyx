cdef extern from "../byteshift_strstr.h":
    char* byteshift_strstr(const char *haystack, const char *needle)

cdef extern from "../byteshift_memmem.h":
    void *byteshift_memmem(const void *haystack, size_t haystack_len,
                           const void *needle, size_t needle_len)


__all__ = ['py_byteshift_strstr', 'py_byteshift_memmem']


def py_byteshift_strstr(haystack, needle):
    cdef char *c_haystack = haystack
    cdef char *c_needle = needle
    cdef char *result
    result = byteshift_strstr(c_haystack, c_needle)
    return (result - c_haystack) if result != NULL else -1

def py_byteshift_memmem(haystack, needle):
    cdef char *c_haystack = haystack
    cdef size_t haystack_len = len(haystack)
    cdef char *c_needle = needle
    cdef size_t needle_len = len(needle)
    cdef void *result
    result = byteshift_memmem(<void *>c_haystack, haystack_len,
                              <void *>c_needle, needle_len)
    return (result - <void *>c_haystack) if result != NULL else -1
