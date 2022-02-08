Information-theoretic predictor

# Installation
CMake and modern C++ compiler are required.

To install itp Python package:
1. Clone itp repository using git;
2. From itp (root) directory issue the following commands:
 
        git submodule sync --recursive
        git submodule update --init

3. From parent directory issue the following command:

        pip install ./itp

To run tests enter itp (root) directory and type

    python setup.py test
