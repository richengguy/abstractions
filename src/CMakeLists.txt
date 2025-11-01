add_subdirectory(abstractions)
add_subdirectory(cli)

if(ABSTRACTIONS_BUILD_GUI)
    log_message("Building graphical frontend")
    add_subdirectory(gui)
endif()

if(ABSTRACTIONS_BUILD_TESTS)
    log_message("Building library tests")
    add_subdirectory(test)
endif()

set(ABSTRACTIONS_INCLUDES ${ABSTRACTIONS_INCLUDES} PARENT_SCOPE)
set(ABSTRACTIONS_INCLUDE_DIR ${ABSTRACTIONS_INCLUDE_DIR} PARENT_SCOPE)
