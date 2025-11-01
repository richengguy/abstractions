find_package(CLI11 REQUIRED)

# Adapted from the webview README.md
include(FetchContent)
FetchContent_Declare(
    webview
    GIT_REPOSITORY https://github.com/webview/webview
    GIT_TAG 55b438dc11d0e46bc68526822380963ae9d362af
)
FetchContent_MakeAvailable(webview)
target_compile_options(webview_core_shared PRIVATE -Wno-deprecated-declarations)
target_compile_options(webview_core_static PRIVATE -Wno-deprecated-declarations)

set(_libraries
    abstractions
    CLI11::CLI11
    webview::core
)

set(_sources
    main.cpp
)

add_executable(abstractions-ui ${_sources})
target_link_libraries(abstractions-ui PRIVATE ${_libraries})
set_target_properties(abstractions-ui
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${abstractions_BINARY_DIR}/bin
)
