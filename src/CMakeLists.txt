add_subdirectory(abstractions)
add_subdirectory(bin)

if(ABSTRACTIONS_BUILD_TESTS)
    log_message("Building library tests")
    add_subdirectory(test)
endif()
