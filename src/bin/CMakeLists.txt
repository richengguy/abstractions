find_package(CLI11 REQUIRED)
find_package(fmt REQUIRED)
find_package(indicators REQUIRED)

add_executable(abstractions-bin
    main.cpp

    support.h
    support.cpp

    generate.h
    generate.cpp

    render.h
    render.cpp
)


target_link_libraries(abstractions-bin
    PRIVATE
    abstractions
    CLI11::CLI11
    indicators::indicators
)
set_target_properties(abstractions-bin
    PROPERTIES
    RUNTIME_OUTPUT_NAME abstractions
    RUNTIME_OUTPUT_DIRECTORY ${abstractions_BINARY_DIR}/bin
)
