cmake_minimum_required(VERSION 3.0)

project(itp_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(SOURCE_DIR src)
set(INCLUDE_DIR include/itp_core)

# Include directory with the headers developed in the scope of that project.
include_directories(${INCLUDE_DIR})

# All source files developed in the scope of that project.
set(PREDICTOR_SOURCES ${SOURCE_DIR}/Compnames.cpp ${SOURCE_DIR}/Sampler.cpp
  ${SOURCE_DIR}/Continuation.cpp ${SOURCE_DIR}/Compressors.cpp ${SOURCE_DIR}/Head.cpp
  ${SOURCE_DIR}/Sdfa.cpp ${SOURCE_DIR}/Automaton.cpp ${SOURCE_DIR}/TableTransformations.cpp
  ${SOURCE_DIR}/NonCompressionAlgorithmAdaptor.cpp ${SOURCE_DIR}/PredictorSubtypes.cpp
  ${SOURCE_DIR}/Predictor.cpp ${SOURCE_DIR}/Selector.cpp)
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
target_compile_options(itp_core PUBLIC -fPIC -Wall -pedantic)

enable_testing()

# find_package(Threads REQUIRED)
add_subdirectory(external/googletest)
set(GTEST_INCLUDE_DIR "external/googletest/googletest/include")
set(GTEST_LIB gtest_main gtest)
set(GMOCK_INCLUDE_DIR "external/googletest/googlemock/include")
set(GMOCK_LIB gmock_main gmock)
include_directories(${GTEST_INCLUDE_DIR})
include_directories(${GMOCK_INCLUDE_DIR})

set(ITP_CORE_TESTS tests/PredictorSubtypesTest.cpp tests/CompressorsTest.cpp tests/BuildersTest.cpp
        tests/TimeSeriesTest.cpp tests/BignumsTest.cpp tests/AutomatonTest.cpp tests/SamplersTest.cpp
        tests/SelectorTest.cpp tests/NonCompressionAlgorithmAdaptorTest.cpp)
add_executable(itp_core_tests ${ITP_CORE_TESTS} ${PREDICTOR_SOURCES} ${PREDICTOR_HEADERS} ${EXTERNAL_HEADERS})
target_link_libraries(itp_core_tests PRIVATE ${COMPRESSION_LIBRARIES} ${GTEST_LIB} ${GMOCK_LIB})
add_test(NAME itp_core_tests COMMAND itp_core_tests)
