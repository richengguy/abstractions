find_package(CLI11 REQUIRED)
find_package(fmt REQUIRED)
find_package(indicators REQUIRED)

if (ABSTRACTIONS_ENABLE_PROFILING)
    find_package(gperftools REQUIRED)
endif()

add_executable(abstractions-bin
    main.cpp

    support.h
    support.cpp

    generate.h
    generate.cpp

    render.h
    render.cpp
)

set(_libraries
    abstractions
    CLI11::CLI11
    indicators::indicators
)

if (ABSTRACTIONS_ENABLE_PROFILING)
    list(APPEND _libraries gperftools::profiler)
endif()

target_link_libraries(abstractions-bin
    PRIVATE
    ${_libraries}
)
set_target_properties(abstractions-bin
    PROPERTIES
    RUNTIME_OUTPUT_NAME abstractions
    RUNTIME_OUTPUT_DIRECTORY ${abstractions_BINARY_DIR}/bin
)
