cmake_minimum_required(VERSION 3.22)
project(abstractions CXX)

function(log_message msg)
    message(STATUS "Abstractions :: ${msg}")
endfunction()

option(ABSTRACTIONS_BUILD_TESTS "Build the application tests." OFF)
option(ABSTRACTIONS_ASSERTS     "Enable the internal library asserts" ON)

set(CMAKE_CXX_STANDARD 23)
add_compile_options(-Wall)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-O0)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-Werror)
endif()

if (ABSTRACTIONS_ASSERTS)
    log_message("Enabling library asserts")
    add_compile_definitions(-DABSTRACTIONS_ENABLE_ASSERTS)

    if (ABSTRACTIONS_BUILD_TESTS)
        log_message("Setting asserts to \"throw only\" mode.")
        add_compile_definitions(-DABSTRACTIONS_ASSERTS_THROW_ONLY)
    endif()
endif()

include(CTest)

add_subdirectory(src)
