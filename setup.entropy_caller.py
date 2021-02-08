from distutils.core import setup
from distutils.extension import Extension
from Cython.Distutils import build_ext

source_files = ['entropy_caller.pyx']
ext_modules = [Extension("entropy_caller",
                         source_files
                         )]

setup(
    name='Entropy app',
    cmdclass={'build_ext': build_ext},
    ext_modules=ext_modules
)
