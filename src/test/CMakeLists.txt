find_package(CLI11)
find_package(fmt)

set(ABSTRACTIONS_TESTS
    test.cpp
)

foreach(test ${ABSTRACTIONS_TESTS})
    cmake_path(GET test STEM _testName)
    add_executable(test-${_testName} ${test})
    target_link_libraries(test-${_testName}
        PRIVATE
        CLI11::CLI11
        fmt::fmt
    )
    set_target_properties(test-${_testName}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${abstractions_BINARY_DIR}/tests
    )
    add_test(NAME ${_testName} COMMAND test-${_testName})
endforeach()
