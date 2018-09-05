from distutils.core import setup
from Cython.Build import cythonize

#python setup.py build_ext --inplace

setup(ext_modules=cythonize("msgpack/_unpacker.pyx"))