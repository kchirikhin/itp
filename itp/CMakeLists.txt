cmake_minimum_required(VERSION 3.0)

project(itp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(SOURCE_DIR predictor)

# Include directory with the headers developed in the scope of that project.
include_directories(${SOURCE_DIR})

# All source files developed in the scope of that project.
set(PREDICTOR_SOURCES ${SOURCE_DIR}/compnames.cc ${SOURCE_DIR}/sampler.cc
  ${SOURCE_DIR}/continuation.cc ${SOURCE_DIR}/compressors.cc ${SOURCE_DIR}/head.cc
  ${SOURCE_DIR}/sdfa.cc ${SOURCE_DIR}/builders.cc ${SOURCE_DIR}/automaton.cc ${SOURCE_DIR}/ttransformations.cc)
file(GLOB PREDICTOR_HEADERS "${SOURCE_DIR}/*.h")

# Include third-party library for high-precision floating-point arithmetic.
include_directories(lib/ttmath)

# Include third-party library for parsing command-line arguments.
include_directories(lib/tclap)

file(GLOB EXTERNAL_HEADERS "lib/ttmath/*.h" "lib/tclap/*.h")

# Include third-party headers and static libraries of compression algorithms.
include_directories(lib/ppmd lib/rp lib/lcacomp lib/zstd lib/bzip2 lib/zpaq)

# Because zlib generates a header.
include_directories(./lib/zlib)
include_directories(${PROJECT_BINARY_DIR}/lib/zlib)

# Add the implementations of the compression algorithms.
add_subdirectory(lib/bzip2)
add_subdirectory(lib/lcacomp)
add_subdirectory(lib/ppmd)
add_subdirectory(lib/rp)
add_subdirectory(lib/zlib)
add_subdirectory(lib/zstd)
add_subdirectory(lib/zpaq)
set(COMPRESSION_LIBRARIES bz2 lca ppmd rp z zstd zpaq)

# Build the Python binding.
add_subdirectory(lib/pybind11)
include_directories(lib/pybind11)
pybind11_add_module(predictor ${SOURCE_DIR}/pyexport.cc ${PREDICTOR_SOURCES}
  ${PREDICTOR_HEADERS} ${EXTERNAL_HEADERS})

target_compile_options(predictor PUBLIC -fPIC -Wall -pedantic)
target_link_libraries(predictor PRIVATE ${COMPRESSION_LIBRARIES})

enable_testing()

# find_package(Threads REQUIRED)
add_subdirectory(lib/googletest)
set(GTEST_INCLUDE_DIR "lib/googletest/googletest/include")
set(GTEST_LIB gtest_main gtest)
set(GMOCK_INCLUDE_DIR "lib/googletest/googlemock/include")
set(GMOCK_LIB gmock_main gmock)
include_directories(${GTEST_INCLUDE_DIR})
include_directories(${GMOCK_INCLUDE_DIR})

set(PREDICTOR_TESTS tests/test_predictor.cc tests/test_compressors.cc tests/test_builders.cc tests/test_tseries.cc)
add_executable(pred_test ${PREDICTOR_TESTS} ${PREDICTOR_SOURCES} ${PREDICTOR_HEADERS}
  ${EXTERNAL_HEADERS})
target_link_libraries(pred_test PRIVATE ${COMPRESSION_LIBRARIES} ${GTEST_LIB} ${GMOCK_LIB} pybind11::embed)
add_test(NAME pred_test COMMAND pred_test)

set(BIGNUMS_TESTS tests/test_bignums.cc)
add_executable(bignums_test ${BIGNUMS_TESTS} ${SOURCE_DIR}/bignums.h)
target_link_libraries(bignums_test ${GTEST_LIB} ${GMOCK_LIB})
add_test(NAME bignums_test COMMAND bignums_test)

set(AUTOMATON_TESTS tests/test_automaton.cc ${SOURCE_DIR}/automaton.cc ${SOURCE_DIR}/sdfa.cc ${SOURCE_DIR}/head.cc)
add_executable(automaton_test ${AUTOMATON_TESTS} ${SOURCE_DIR}/automaton.h ${SOURCE_DIR}/sdfa.h
  ${SOURCE_DIR}/head.h)
target_link_libraries(automaton_test ${GTEST_LIB} ${GMOCK_LIB})
add_test(NAME automaton_test COMMAND automaton_test)

set(SAMPLERS_TESTS tests/test_samplers.cc ${SOURCE_DIR}/sampler.cc)
add_executable(samplers_tests ${SAMPLERS_TESTS} ${PREDICTOR_SOURCES} ${PREDICTOR_HEADERS} ${EXTERNAL_HEADERS})
target_link_libraries(samplers_tests PRIVATE ${COMPRESSION_LIBRARIES} ${GTEST_LIB} ${GMOCK_LIB})
add_test(NAME samplers_tests COMMAND samplers_tests)

set(SELECTOR_TESTS tests/test_selector.cc ${SOURCE_DIR}/sampler.cc)
add_executable(selector_test ${SELECTOR_TESTS} ${SOURCE_DIR}/selector.h)
target_link_libraries(selector_test ${GTEST_LIB} ${GMOCK_LIB})
add_test(NAME selector_test COMMAND selector_test)

set(NON_COMPRESSION_ALGORITHM_ADAPTER_TESTS tests/test_non_compression_algorithm_adaptor.cc ${SOURCE_DIR}/NonCompressionAlgorithmAdaptor.cc)
add_executable(non_compression_algorithm_adaptor_test ${NON_COMPRESSION_ALGORITHM_ADAPTER_TESTS})
target_link_libraries(non_compression_algorithm_adaptor_test ${GTEST_LIB} ${GMOCK_LIB})
add_test(NAME non_compression_algorithm_adaptor_test COMMAND non_compression_algorithm_adaptor_test)
