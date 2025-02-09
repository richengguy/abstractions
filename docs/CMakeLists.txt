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

set(ABSTRACTIONS_DOCS ${abstractions_SOURCE_DIR}/docs)
set(ABSTRACTIONS_DOCS_OUTPUT ${abstractions_BINARY_DIR}/docs)

set(ABSTRACTIONS_DOXYGEN_PROJECT ${PROJECT_NAME})
set(ABSTRACTIONS_DOXYGEN_INPUT ${ABSTRACTIONS_INCLUDE_DIR})
set(ABSTRACTIONS_DOXYGEN_OUTPUT ${ABSTRACTIONS_DOCS_OUTPUT}/doxygen)
set(ABSTRACTIONS_DOXYGEN_XML ${ABSTRACTIONS_DOXYGEN_OUTPUT}/xml)
set(ABSTRACTIONS_DOXYFILE ${ABSTRACTIONS_DOXYGEN_OUTPUT}/Doxyfile)

set(ABSTRACTIONS_SPHINX_CONF_PY ${ABSTRACTIONS_DOCS}/conf.py)
set(ABSTRACTIONS_SPHINX_OUTPUT ${ABSTRACTIONS_DOCS_OUTPUT}/sphinx)

set(ABSTRACTIONS_RST_FILES
    ${ABSTRACTIONS_DOCS}/index.md

    ${ABSTRACTIONS_DOCS}/abstracting-images.md
    ${ABSTRACTIONS_DOCS}/getting-started.md

    ${ABSTRACTIONS_DOCS}/api/api.rst
    ${ABSTRACTIONS_DOCS}/api/engine.rst
    ${ABSTRACTIONS_DOCS}/api/images.rst
    ${ABSTRACTIONS_DOCS}/api/pgpe.rst
    ${ABSTRACTIONS_DOCS}/api/types.rst
)

configure_file(${abstractions_SOURCE_DIR}/docs/Doxyfile.in ${ABSTRACTIONS_DOXYFILE} @ONLY)

# Creates the Doxygen XML files needed to generate the Sphinx docs.
add_custom_command(
    OUTPUT ${ABSTRACTIONS_DOXYGEN_XML}
    DEPENDS ${ABSTRACTIONS_INCLUDES}
            ${abstractions_SOURCE_DIR}/docs/Doxyfile.in
    COMMAND ${DOXYGEN_EXECUTABLE} ${ABSTRACTIONS_DOXYFILE}
    COMMENT "Generating Doxygen XML files."
)

# Creates the Sphinx documention using sphinx-build and the Doxygen XML files.
add_custom_command(
    OUTPUT ${ABSTRACTIONS_SPHINX_OUTPUT}
    COMMAND
        ${SPHINX_EXECUTABLE}
        -b html
        -Dbreathe_projects.abstractions=${ABSTRACTIONS_DOXYGEN_XML}
        ${abstractions_SOURCE_DIR}/docs
        ${ABSTRACTIONS_SPHINX_OUTPUT}
    WORKING_DIRECTORY ${abstractions_BINARY_DIR}
    DEPENDS ${ABSTRACTIONS_RST_FILES} ${ABSTRACTIONS_DOXYGEN_XML} ${ABSTRACTIONS_SPHINX_CONF_PY}
    COMMENT "Generating Sphinx docs."
)

add_custom_target(docs
    DEPENDS ${ABSTRACTIONS_SPHINX_OUTPUT}
    COMMENT "Building project documentation."
)
