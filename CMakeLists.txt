CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(MLL)

# 0. Environment
INCLUDE_DIRECTORIES(${MLL_SOURCE_DIR}/lib)

# 1. Load external libraries
ADD_SUBDIRECTORY(lib/gtest)
ADD_SUBDIRECTORY(lib/sqlite)

# 2. Load own source
ADD_SUBDIRECTORY(src)

########################### Build documentation with Doxygen ###########################
#INCLUDE(${CMAKE_ROOT}/Modules/Documentation.cmake OPTIONAL)
#OPTION(BUILD_DOCUMENTATION "Build MachineLearningLibrary reference documentation using doxygen (use: make DoxygenDoc)" OFF)
#MARK_AS_ADVANCED(CLEAR BUILD_DOCUMENTATION)

# This processes our doxyfile.cmake and substitutes paths to generate
# a final Doxyfile
#CONFIGURE_FILE(${MachineLearningLibrary_SOURCE_DIR}/doc/doxyfile.cmake
#	${MachineLearningLibrary_BINARY_DIR}/doxyfile.cmake
#)    

# Searching for doxygen location and initializes ${DOXYGEN} when found
#FIND_PACKAGE(DOXYGEN)

#INSTALL(DIRECTORY ${MachineLearningLibrary_BINARY_DIR} DESTINATION ${CMAKE_INSTALL_PREFIX})

# This creates a new target to build documentation.
# It runs ${DOXYGEN} which is the full path and executable to
# Doxygen on your system, set by the FindDoxygen.cmake module
# (called by FindDocumentation.cmake).
# It runs the final generated Doxyfile against it.
# The DOT_PATH is substituted into the Doxyfile.
#ADD_CUSTOM_TARGET(doc ${DOXYGEN}
#	${MachineLearningLibrary_BINARY_DIR}/doxyfile.cmake
#)
