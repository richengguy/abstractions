find_package(blend2d)
find_package(Eigen3)
find_package(fmt)

set(ABSTRACTIONS_INCLUDE_DIR ${abstractions_SOURCE_DIR}/include/abstractions)

set(ABSTRACTIONS_INCLUDES
    ${ABSTRACTIONS_INCLUDE_DIR}/abstractions.h
    ${ABSTRACTIONS_INCLUDE_DIR}/pgpe.h
    ${ABSTRACTIONS_INCLUDE_DIR}/profile.h
    ${ABSTRACTIONS_INCLUDE_DIR}/types.h
    ${ABSTRACTIONS_INCLUDE_DIR}/utilities.h

    ${ABSTRACTIONS_INCLUDE_DIR}/math/matrices.h
    ${ABSTRACTIONS_INCLUDE_DIR}/math/random.h
    ${ABSTRACTIONS_INCLUDE_DIR}/math/types.h
)

set (ABSTRACTIONS_SOURCES
    abstractions.cpp
    pgpe.cpp
)

add_library(abstractions ${ABSTRACTIONS_INCLUDES} ${ABSTRACTIONS_SOURCES})

target_compile_features(abstractions PUBLIC cxx_std_23)

target_link_libraries(abstractions
    PUBLIC
    blend2d::blend2d
    Eigen3::Eigen
    fmt::fmt
)

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
