from distutils.core import setup, Extension

cython_wrappers_module = Extension(
    'cython_wrappers',
    sources=['cython_wrappers.c', '../byteshift_strstr.c', '../byteshift_memmem.c'],
    include_dirs=['../'],
)

setup(
    ext_modules = [cython_wrappers_module],
)
