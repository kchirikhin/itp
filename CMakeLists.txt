cmake_minimum_required(VERSION 3.0)

project(itp_core_bindings LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(SOURCE_DIR src)

set(ITP_CORE_BINDINGS_SOURCES ${SOURCE_DIR}/itp_core_bindings.cc)

# Build the Python binding.
#find_package(Python COMPONENTS Interpreter Development REQUIRED)
#if (NOT ${Python_FOUND})
#    message(FATAL_ERROR "Cannot find python components")
#endif()

add_subdirectory(external/pybind11)
include_directories(external/pybind11/include)

add_subdirectory(lib/itp_core)
include_directories(lib/itp_core/include)

pybind11_add_module(itp_core_bindings ${ITP_CORE_BINDINGS_SOURCES})

target_compile_options(itp_core_bindings PUBLIC -fPIC -Wall -pedantic)
target_link_libraries(itp_core_bindings PRIVATE ${COMPRESSION_LIBRARIES} itp_core)