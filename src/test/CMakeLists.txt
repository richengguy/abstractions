find_package(doctest REQUIRED)
find_package(fmt REQUIRED)

set(ABSTRACTIONS_TESTS
    library-tests.cpp
    math.cpp
    pgpe.cpp
    profile.cpp
    utilities.cpp
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
