find_package(CLI11 REQUIRED)
find_package(doctest REQUIRED)
find_package(fmt REQUIRED)

configure_file(
    test-paths.h.in
    ${abstractions_BINARY_DIR}/tests/test-paths.h
)

# 'abstractions' library tests
set(ABSTRACTIONS_TESTS
    library/main.cpp

    library/errors.cpp
    library/image.cpp
    library/math.cpp
    library/pgpe.cpp
    library/profile.cpp
    library/render.cpp
    library/shapes.cpp
    library/threads.cpp
    library/types.cpp

    ${abstractions_BINARY_DIR}/tests/test-paths.h
    ${abstractions_SOURCE_DIR}/src/test/library/support.h
)

add_executable(library-tests
    ${ABSTRACTIONS_TESTS}
    ${abstractions_BINARY_DIR}/tests/samples
)
target_include_directories(library-tests
    PRIVATE
    ${abstractions_SOURCE_DIR}/src/test/library
    ${abstractions_BINARY_DIR}/tests
)
target_link_libraries(library-tests
    PRIVATE
    abstractions
    doctest::doctest
)
set_target_properties(library-tests
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${abstractions_BINARY_DIR}/tests
)
add_custom_command(
    OUTPUT ${abstractions_BINARY_DIR}/tests/samples
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/samples
        ${abstractions_BINARY_DIR}/tests/samples
)

include(doctest)
doctest_discover_tests(library-tests)

# Specific feature tests
function(add_feature_test featureName)
    log_message("Adding feature test - ${featureName}")
    add_executable(${featureName}-test
        feature/${featureName}.cpp
        ${abstractions_SOURCE_DIR}/src/test/feature/support.h
    )
    target_include_directories(${featureName}-test
        PRIVATE
        ${abstractions_SOURCE_DIR}/src/test/feature
        ${abstractions_BINARY_DIR}/tests
    )
    target_link_libraries(${featureName}-test
        PRIVATE
        abstractions
        CLI11::CLI11
    )
    set_target_properties(${featureName}-test
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${abstractions_BINARY_DIR}/tests
    )
endfunction()

add_feature_test(assert)
add_feature_test(canvas)
add_feature_test(threads)
