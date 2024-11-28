add_executable(abstractions-bin main.cpp)
target_link_libraries(abstractions-bin
    PRIVATE
    abstractions
)
set_target_properties(abstractions-bin
    PROPERTIES
    RUNTIME_OUTPUT_NAME abstractions
    RUNTIME_OUTPUT_DIRECTORY ${abstractions_BINARY_DIR}/bin
)
