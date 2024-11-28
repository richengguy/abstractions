set(ABSTRACTIONS_INCLUDE_DIR ${abstractions_SOURCE_DIR}/include/abstractions)

set(ABSTRACTIONS_INCLUDES
    ${ABSTRACTIONS_INCLUDE_DIR}/abstractions.h
)

set (ABSTRACTIONS_SOURCES
    abstractions.cpp
)

add_library(abstractions ${ABSTRACTIONS_INCLUDES} ${ABSTRACTIONS_SOURCES})
target_compile_features(abstractions PUBLIC cxx_std_20)
target_include_directories(abstractions
    PUBLIC ${abstractions_SOURCE_DIR}/include
           ${abstractions_BINARY_DIR}/include
    PRIVATE ${abstractions_SOURCE_DIR}/src/abstractions
)
set_target_properties(abstractions
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${abstractions_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${abstractions_BINARY_DIR}/lib
)
