# This CMake file has been adapted from
# https://devblogs.microsoft.com/cppblog/clear-functional-c-documentation-with-sphinx-breathe-doxygen-cmake/

find_package(Doxygen REQUIRED)

find_program(
    SPHINX_EXECUTABLE
    NAMES sphinx-build
    DOC "Path to sphinx-build."
)

if (SPHINX_EXECUTABLE)
    log_message("Found sphinx-build.")
else()
    message(FATAL_ERROR "Could not find sphinx-build; cannot generate documentation.")
endif()

set(ABSTRACTIONS_DOCS_OUTPUT ${abstractions_BINARY_DIR}/docs)

set(ABSTRACTIONS_DOXYGEN_PROJECT ${PROJECT_NAME})
set(ABSTRACTIONS_DOXYGEN_INPUT ${ABSTRACTIONS_INCLUDE_DIR})
set(ABSTRACTIONS_DOXYGEN_OUTPUT ${ABSTRACTIONS_DOCS_OUTPUT}/doxygen)
set(ABSTRACTIONS_DOXYGEN_INDEX_HTML ${ABSTRACTIONS_DOXYGEN_OUTPUT}/html/index.html)
set(ABSTRACTIONS_DOXYFILE ${ABSTRACTIONS_DOCS_OUTPUT}/Doxyfile)
set(ABSTRACTIONS_SPHINX_OUTPUT ${ABSTRACTIONS_DOCS_OUTPUT}/sphinx)

configure_file(${abstractions_SOURCE_DIR}/docs/Doxyfile.in ${ABSTRACTIONS_DOXYFILE} @ONLY)

add_custom_command(
    OUTPUT ${ABSTRACTIONS_DOXYGEN_INDEX_HTML}
    DEPENDS ${ABSTRACTIONS_INCLUDES}
            ${abstractions_SOURCE_DIR}/docs/Doxyfile.in
    COMMAND ${DOXYGEN_EXECUTABLE} ${ABSTRACTIONS_DOXYFILE}
    COMMENT "Generating Doxygen docs"
)

add_custom_target(Doxygen ALL DEPENDS ${ABSTRACTIONS_DOXYGEN_INDEX_HTML})
add_custom_target(Sphinx ALL
    COMMAND
        ${SPHINX_EXECUTABLE}
        -b html
        -Dbreathe_projects.abstractions=${ABSTRACTIONS_DOXYGEN_OUTPUT}/xml
        ${abstractions_SOURCE_DIR}/docs
        ${ABSTRACTIONS_SPHINX_OUTPUT}
    WORKING_DIRECTORY ${abstractions_BINARY_DIR}
    COMMENT "Generating Sphinx docs"
)
