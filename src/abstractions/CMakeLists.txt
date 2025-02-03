find_package(blend2d REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(fmt REQUIRED)
find_package(nlohmann_json REQUIRED)

set(ABSTRACTIONS_INCLUDE_DIR ${abstractions_SOURCE_DIR}/include/abstractions PARENT_SCOPE)

set(ABSTRACTIONS_INCLUDES
    ${ABSTRACTIONS_INCLUDE_DIR}/abstractions.h
    ${ABSTRACTIONS_INCLUDE_DIR}/engine.h
    ${ABSTRACTIONS_INCLUDE_DIR}/errors.h
    ${ABSTRACTIONS_INCLUDE_DIR}/image.h
    ${ABSTRACTIONS_INCLUDE_DIR}/pgpe.h
    ${ABSTRACTIONS_INCLUDE_DIR}/profile.h
    ${ABSTRACTIONS_INCLUDE_DIR}/types.h

    ${ABSTRACTIONS_INCLUDE_DIR}/math/matrices.h
    ${ABSTRACTIONS_INCLUDE_DIR}/math/random.h
    ${ABSTRACTIONS_INCLUDE_DIR}/math/types.h

    ${ABSTRACTIONS_INCLUDE_DIR}/render/canvas.h
    ${ABSTRACTIONS_INCLUDE_DIR}/render/renderer.h
    ${ABSTRACTIONS_INCLUDE_DIR}/render/shapes.h

    ${ABSTRACTIONS_INCLUDE_DIR}/threads/job.h
    ${ABSTRACTIONS_INCLUDE_DIR}/threads/queue.h
    ${ABSTRACTIONS_INCLUDE_DIR}/threads/threadpool.h
    ${ABSTRACTIONS_INCLUDE_DIR}/threads/worker.h

    ${ABSTRACTIONS_INCLUDE_DIR}/terminal/chrono.h
    ${ABSTRACTIONS_INCLUDE_DIR}/terminal/console.h
    ${ABSTRACTIONS_INCLUDE_DIR}/terminal/table.h

    PARENT_SCOPE
)

set (ABSTRACTIONS_SOURCES
    engine.cpp
    errors.cpp
    image.cpp
    json.h
    json.cpp
    pgpe.cpp

    render/canvas.cpp
    render/renderer.cpp
    render/shapes.cpp

    threads/job.cpp
    threads/queue.cpp
    threads/threadpool.cpp
    threads/worker.cpp

    terminal/chrono.cpp
    terminal/console.cpp
    terminal/table.cpp
)

add_library(abstractions ${ABSTRACTIONS_INCLUDES} ${ABSTRACTIONS_SOURCES})

target_compile_features(abstractions PUBLIC cxx_std_23)

target_link_libraries(abstractions
    PUBLIC
    blend2d::blend2d
    Eigen3::Eigen
    fmt::fmt
    nlohmann_json::nlohmann_json
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
