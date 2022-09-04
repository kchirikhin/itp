import os
import re
import sys
import platform
import subprocess
import unittest

from setuptools import setup, Extension, find_packages
from setuptools.command.build_ext import build_ext
from distutils.version import LooseVersion
from shutil import copyfile, copymode


def my_test_suite():
    test_loader = unittest.TestLoader()
    test_suite = test_loader.discover('tests', pattern='test_*.py')
    return test_suite


# Adapted from:
# 1. https://www.benjack.io/2018/02/02/python-cpp-revisited.html
# 2. https://python.plainenglish.io/building-hybrid-python-c-packages-8985fa1c5b1d
class CMakeExtension(Extension):
    def __init__(self, name, output_dir='', source_dir=''):
        Extension.__init__(self, name, sources=[])
        self.output_dir = output_dir
        self.source_dir = os.path.abspath(source_dir)


class CMakeBuild(build_ext):
    def run(self):
        try:
            out = subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError("CMake must be installed to build the following extensions: " +
                               ", ".join(e.name for e in self.extensions))

        if platform.system() == "Windows":
            cmake_version = LooseVersion(re.search(r'version\s*([\d.]+)', out.decode()).group(1))
            if cmake_version < '3.1.0':
                raise RuntimeError("CMake >= 3.1.0 is required on Windows")

        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        ext_name = ext.name
        if ext.output_dir:
            ext_name = os.path.join(ext.output_dir, ext_name)
        ext_dir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext_name)))
        print(f"{ext_dir=}")
        cmake_args = ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=' + ext_dir,
                      '-DPYTHON_EXECUTABLE=' + sys.executable]

        cfg = 'Debug' if self.debug else 'Release'
        build_args = ['--config', cfg]

        if platform.system() == "Windows":
            cmake_args += ['-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}'.format(cfg.upper(), ext_dir)]
            if sys.maxsize > 2**32:
                cmake_args += ['-A', 'x64']
            build_args += ['--', '/m']
        else:
            cmake_args += ['-DCMAKE_BUILD_TYPE=' + cfg]
            build_args += ['--', '-j2']

        env = os.environ.copy()
        env['CXXFLAGS'] = '{} -DVERSION_INFO=\\"{}\\"'.format(env.get('CXXFLAGS', ''),
                                                              self.distribution.get_version())
        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)
        subprocess.check_call(['cmake', ext.source_dir] + cmake_args, cwd=self.build_temp, env=env)
        subprocess.check_call(['cmake', '--build', '.'] + build_args, cwd=self.build_temp)

        # Copy itp_core_tests to tests directory.
        test_bin = os.path.join(self.build_temp, 'lib', 'itp_core', 'itp_core_tests')
        self.copy_test_file(test_bin)
        print()

    @staticmethod
    def copy_test_file(src_file):
        """
        Copy ``src_file`` to `tests/bin` directory, ensuring parent directory
        exists. Messages like `creating directory /path/to/package` and
        `copying directory /src/path/to/package -> path/to/package` are
        displayed on standard output. Adapted from scikit-build.
        """
        # Create directory if needed.
        dest_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'tests', 'bin')
        if dest_dir != "" and not os.path.exists(dest_dir):
            print("Creating directory {}...".format(dest_dir))
            os.makedirs(dest_dir)

        # Copy file.
        dest_file = os.path.join(dest_dir, os.path.basename(src_file))
        print("Copying {} -> {}...".format(src_file, dest_file))
        copyfile(src_file, dest_file)
        copymode(src_file, dest_file)


setup(
    name='itp',
    version='0.0.4',
    author='Konstantin Chirikhin',
    author_email='chirihin@gmail.com',
    description='An information-theoretic predictor for time series',
    long_description='',
    packages=find_packages(exclude=["tests"]),
    package_data={'': ['tests/*.dat']},
    install_requires=['numpy', 'pandas', 'matplotlib', 'statsmodels'],
    ext_modules=[CMakeExtension('itp_core_bindings', 'itp')],
    cmdclass=dict(build_ext=CMakeBuild),
    zip_safe=False,
    test_suite='tests',
)
