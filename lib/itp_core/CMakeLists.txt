cmake_minimum_required(VERSION 3.0)

project(itp_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(SOURCE_DIR src)
set(INCLUDE_DIR include/itp_core)

# Include directory with the headers developed in the scope of that project.
include_directories(${INCLUDE_DIR})

# All source files developed in the scope of that project.
set(PREDICTOR_SOURCES ${SOURCE_DIR}/compnames.cc ${SOURCE_DIR}/sampler.cc
  ${SOURCE_DIR}/continuation.cc ${SOURCE_DIR}/compressors.cc ${SOURCE_DIR}/head.cc
  ${SOURCE_DIR}/sdfa.cc ${SOURCE_DIR}/automaton.cc ${SOURCE_DIR}/ttransformations.cc
  ${SOURCE_DIR}/NonCompressionAlgorithmAdaptor.cc ${SOURCE_DIR}/NonCompressionAlgorithmAdaptor.cc
  ${SOURCE_DIR}/predictor_subtypes.cc ${SOURCE_DIR}/predictor.cc)
file(GLOB PREDICTOR_HEADERS "${SOURCE_DIR}/*.h" "${INCLUDE_DIR}/*.h")

# Include third-party library for high-precision floating-point arithmetic.
include_directories(external/ttmath)

# Include third-party library for parsing command-line arguments.
include_directories(external/tclap)

file(GLOB EXTERNAL_HEADERS "external/ttmath/*.h" "external/tclap/*.h")

# Include third-party headers and static libraries of compression algorithms.
include_directories(external/ppmd external/rp external/lcacomp external/zstd external/bzip2 external/zpaq)

# Because zlib generates a header.
include_directories(external/zlib)
include_directories(${PROJECT_BINARY_DIR}/external/zlib)

# Add the implementations of the compression algorithms.
add_subdirectory(external/bzip2)
add_subdirectory(external/lcacomp)
add_subdirectory(external/ppmd)
add_subdirectory(external/rp)
add_subdirectory(external/zlib)
add_subdirectory(external/zstd)
add_subdirectory(external/zpaq)

set(COMPRESSION_LIBRARIES bz2 lca ppmd rp z zstd zpaq)

get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if (HAS_PARENT)
    set(COMPRESSION_LIBRARIES ${COMPRESSION_LIBRARIES} PARENT_SCOPE)
endif()

add_library(itp_core STATIC ${PREDICTOR_SOURCES})

enable_testing()

# find_package(Threads REQUIRED)
add_subdirectory(external/googletest)
set(GTEST_INCLUDE_DIR "external/googletest/googletest/include")
set(GTEST_LIB gtest_main gtest)
set(GMOCK_INCLUDE_DIR "external/googletest/googlemock/include")
set(GMOCK_LIB gmock_main gmock)
include_directories(${GTEST_INCLUDE_DIR})
include_directories(${GMOCK_INCLUDE_DIR})

set(ITP_CORE_TESTS tests/test_predictor_subtypes.cc tests/test_compressors.cc tests/test_builders.cc tests/test_tseries.cc)
add_executable(itp_core_tests ${PREDICTOR_TESTS} ${PREDICTOR_SOURCES} ${PREDICTOR_HEADERS} ${EXTERNAL_HEADERS})
target_link_libraries(itp_core_tests PRIVATE ${COMPRESSION_LIBRARIES} ${GTEST_LIB} ${GMOCK_LIB})
add_test(NAME itp_core_tests COMMAND itp_core_tests)

set(BIGNUMS_TESTS tests/test_bignums.cc)
add_executable(bignums_tests ${BIGNUMS_TESTS} ${SOURCE_DIR}/bignums.h)
target_link_libraries(bignums_tests ${GTEST_LIB} ${GMOCK_LIB})
add_test(NAME bignums_tests COMMAND bignums_tests)

set(AUTOMATON_TESTS tests/test_automaton.cc ${SOURCE_DIR}/automaton.cc ${SOURCE_DIR}/sdfa.cc ${SOURCE_DIR}/head.cc)
add_executable(automaton_tests ${AUTOMATON_TESTS} ${SOURCE_DIR}/automaton.h ${SOURCE_DIR}/sdfa.h
  ${SOURCE_DIR}/head.h)
target_link_libraries(automaton_tests ${GTEST_LIB} ${GMOCK_LIB})
add_test(NAME automaton_tests COMMAND automaton_tests)

set(SAMPLERS_TESTS tests/test_samplers.cc ${SOURCE_DIR}/sampler.cc)
add_executable(samplers_tests ${SAMPLERS_TESTS} ${PREDICTOR_SOURCES} ${PREDICTOR_HEADERS} ${EXTERNAL_HEADERS})
target_link_libraries(samplers_tests PRIVATE ${COMPRESSION_LIBRARIES} ${GTEST_LIB} ${GMOCK_LIB})
add_test(NAME samplers_tests COMMAND samplers_tests)

set(SELECTOR_TESTS tests/test_selector.cc ${SOURCE_DIR}/sampler.cc ${SOURCE_DIR}/continuation.cc)
add_executable(selector_tests ${SELECTOR_TESTS} ${INCLUDE_DIR}/selector.h)
target_link_libraries(selector_tests ${GTEST_LIB} ${GMOCK_LIB})
add_test(NAME selector_tests COMMAND selector_tests)

set(NON_COMPRESSION_ALGORITHM_ADAPTER_TESTS tests/test_non_compression_algorithm_adaptor.cc ${SOURCE_DIR}/NonCompressionAlgorithmAdaptor.cc)
add_executable(non_compression_algorithm_adaptor_tests ${NON_COMPRESSION_ALGORITHM_ADAPTER_TESTS})
target_link_libraries(non_compression_algorithm_adaptor_tests ${GTEST_LIB} ${GMOCK_LIB})
add_test(NAME non_compression_algorithm_adaptor_tests COMMAND non_compression_algorithm_adaptor_tests)
