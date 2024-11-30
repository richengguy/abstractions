add_subdirectory(abstractions)
add_subdirectory(bin)

if(ABSTRACTIONS_BUILD_TESTS)
    add_subdirectory(test)
endif()
