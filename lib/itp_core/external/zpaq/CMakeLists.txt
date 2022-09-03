cmake_minimum_required(VERSION 3.5)

project(zpaq)

find_package(Threads)

set(ZPAQ_SOURCES libzpaq.cpp)
set(ZPAQ_HEADERS libzpaq.h)

add_library(zpaq STATIC ${ZPAQ_SOURCES} ${ZPAQ_HEADERS})
set_target_properties(zpaq PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
target_link_libraries(zpaq ${CMAKE_THREADS_LIBS_INIT})
