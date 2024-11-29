cmake_minimum_required(VERSION 3.15)
project(abstractions CXX)

option(ABSTRACTIONS_BUILD_TESTS "Build the application tests." OFF)

set(CMAKE_CXX_STANDARD 20)
add_compile_options(-Wall)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-O0)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-Werror)
endif()

include(CTest)

add_subdirectory(src)
