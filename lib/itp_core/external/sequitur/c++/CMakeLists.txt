cmake_minimum_required(VERSION 3.0)
project(sequitur LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

set(SEQUITUR_SOURCES sequitur.cc classes.cc compress.cc arith.c bitio.c stats.c)

add_library(sequitur STATIC ${SEQUITUR_SOURCES})
set_target_properties(sequitur PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
