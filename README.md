Information-theoretic predictor

# Installation
CMake and modern C++ compiler are required. Tested with CMake version 3.24.1, g++ 7.5.0 and Python 3.10.


To install itp Python package:
1. Clone itp repository using git:

        git clone https://github.com/kchirikhin/itp.git

2. From itp (root) directory issue the following commands:
 
        git submodule sync --recursive
        git submodule update --init

3. Optionally, create a virtual environment:

        python -m venv venv
        source ./venv/bin/activate

4. From parent directory issue the following command:

        pip install ./itp

To run tests enter itp (root) directory and type

        python setup.py test
