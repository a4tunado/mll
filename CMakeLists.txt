CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(MLL)

# 0. Environment
INCLUDE_DIRECTORIES(${MLL_SOURCE_DIR}/lib)

# 1. Load external libraries
ADD_SUBDIRECTORY(lib/gtest)

# 2. Load own source
ADD_SUBDIRECTORY(src)

# 3. Doxygen
INCLUDE(${CMAKE_ROOT}/Modules/Documentation.cmake OPTIONAL)
OPTION(BUILD_DOCUMENTATION "Build MLL documentation using doxygen (use: make DoxygenDoc)" OFF)

FIND_PACKAGE(Doxygen)
IF(DOXYGEN_FOUND)
    CONFIGURE_FILE(${MLL_SOURCE_DIR}/doc/doxyfile.cmake ${MLL_BINARY_DIR}/doxyfile.cmake)
    ADD_CUSTOM_TARGET(reference ${DOXYGEN} ${MLL_BINARY_DIR}/doxyfile.cmake)
ENDIF()

