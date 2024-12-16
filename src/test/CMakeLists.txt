find_package(doctest REQUIRED)
find_package(fmt REQUIRED)

# Top-level Tests
set(ABSTRACTIONS_TESTS
    library-tests.cpp
    errors.cpp
    math.cpp
    pgpe.cpp
    profile.cpp
)

add_executable(library-tests ${ABSTRACTIONS_TESTS})
target_link_libraries(library-tests
    PRIVATE
    abstractions
    doctest::doctest
)
set_target_properties(library-tests
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${abstractions_BINARY_DIR}/tests
)

include(doctest)
doctest_discover_tests(library-tests)

# Specific feature tests
add_executable(assert-test assert-test.cpp)
target_link_libraries(assert-test
    PRIVATE
    abstractions
)
set_target_properties(assert-test
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${abstractions_BINARY_DIR}/tests
)
