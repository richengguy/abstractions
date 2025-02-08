# This CMake file has been adapted from
# https://devblogs.microsoft.com/cppblog/clear-functional-c-documentation-with-sphinx-breathe-doxygen-cmake/

find_package(Doxygen REQUIRED)

set(ABSTRACTIONS_DOCS_OUTPUT ${abstractions_BINARY_DIR}/docs)

set(ABSTRACTIONS_DOXYGEN_PROJECT ${PROJECT_NAME})
set(ABSTRACTIONS_DOXYGEN_INPUT ${ABSTRACTIONS_INCLUDE_DIR})
set(ABSTRACTIONS_DOXYGEN_OUTPUT ${ABSTRACTIONS_DOCS_OUTPUT}/doxygen)
set(ABSTRACTIONS_DOXYGEN_INDEX_HTML ${ABSTRACTIONS_DOXYGEN_OUTPUT}/html/index.html)
set(ABSTRACTIONS_DOXYFILE ${ABSTRACTIONS_DOCS_OUTPUT}/Doxyfile)

configure_file(${abstractions_SOURCE_DIR}/docs/Doxyfile.in ${ABSTRACTIONS_DOXYFILE} @ONLY)

add_custom_command(
    OUTPUT ${ABSTRACTIONS_DOXYGEN_INDEX_HTML}
    DEPENDS ${ABSTRACTIONS_INCLUDES}
            ${abstractions_SOURCE_DIR}/docs/Doxyfile.in
    COMMAND ${DOXYGEN_EXECUTABLE} ${ABSTRACTIONS_DOXYFILE}
    COMMENT "Generating docs"
)

add_custom_target(Doxygen ALL DEPENDS ${ABSTRACTIONS_DOXYGEN_INDEX_HTML})
