cmake_minimum_required(VERSION 3.22)
project(abstractions CXX)

function(log_message msg)
    message(STATUS "Abstractions :: ${msg}")
endfunction()

option(ABSTRACTIONS_BUILD_TESTS         "Build the application tests."       OFF)
option(ABSTRACTIONS_ASSERTS             "Enable the internal library asserts" ON)
option(ABSTRACTIONS_ENABLE_ASAN         "Enable AddressSanitizer with build" OFF)
option(ABSTRACTIONS_ENABLE_PROFILING    "Enabled profiler support."          OFF)

set(CMAKE_CXX_STANDARD 23)
add_compile_options(-Wall)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-O0)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-Werror)
endif()

if (ABSTRACTIONS_ASSERTS)
    log_message("Enabling library asserts")
    add_compile_definitions(ABSTRACTIONS_ENABLE_ASSERTS)

    if (ABSTRACTIONS_BUILD_TESTS)
        log_message("Setting asserts to \"throw only\" mode.")
        add_compile_definitions(ABSTRACTIONS_ASSERTS_THROW_ONLY)
    endif()
endif()

if (ABSTRACTIONS_ENABLE_ASAN)
    log_message("Enabling AddressSanitizer")
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

if (ABSTRACTIONS_ENABLE_PROFILING)
    log_message("Enabling profiling via gperftools")
    add_compile_definitions(ABSTRACTIONS_ENABLE_GPERFTOOLS)
endif()

include(CTest)

add_subdirectory(src)
