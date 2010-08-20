CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(MachineLearningLibrary)

FILE(GLOB_RECURSE CORE_HDR_FILES ${MachineLearningLibrary_SOURCE_DIR}/core/*.h)

FILE(GLOB_RECURSE CORE_SRC_FILES
    PATTERN 
    "${MachineLearningLibrary_SOURCE_DIR}/core/*.cpp")
    
FILE(GLOB_RECURSE CORE_TEST_SRC_FILES ${MachineLearningLibrary_SOURCE_DIR}/core/*_ut.cpp)

LIST(REMOVE_ITEM CORE_SRC_FILES CORE_TEST_SRC_FILES)

FILE(GLOB_RECURSE CLASSIFIERS_HDR_FILES ${MachineLearningLibrary_SOURCE_DIR}/classifiers/*.h)

FILE(GLOB_RECURSE CLASSIFIERS_SRC_FILES 
    PATTERN 
    "${MachineLearningLibrary_SOURCE_DIR}/classifiers/*.cpp")

FILE(GLOB_RECURSE CLASSIFIERS_TEST_SRC_FILES ${MachineLearningLibrary_SOURCE_DIR}/classifiers/*_ut.cpp)

LIST(REMOVE_ITEM CLASSIFIERS_SRC_FILES CLASSIFIERS_TEST_SRC_FILES)
  
FILE(GLOB_RECURSE MAIN_SRC_FILES ${MachineLearningLibrary_SOURCE_DIR}/core/main.cpp)

LIST(REMOVE_ITEM CORE_SRC_FILES MAIN_SRC_FILES)

SET(GTEST_SRC_FILES
	${MachineLearningLibrary_SOURCE_DIR}/libs/gtest/src/gtest_main.cc
	${MachineLearningLibrary_SOURCE_DIR}/libs/gtest/src/gtest-all.cc
)

SET(CMAKE_DEBUG_POSTFIX "d")

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)


# Stuff that should be in every build...
INCLUDE_DIRECTORIES(
	${MachineLearningLibrary_SOURCE_DIR}/core/
	${MachineLearningLibrary_SOURCE_DIR}/libs/gtest/
	${MachineLearningLibrary_SOURCE_DIR}/libs/gtest/include/
	${CMAKE_INCLUDE_PATH}
)

LINK_DIRECTORIES(
	${CMAKE_LIBRARY_PATH}
)

LINK_LIBRARIES(
)

########################### mll ###########################

ADD_LIBRARY(mll STATIC	${CORE_HDR_FILES}
						${CORE_SRC_FILES}
)
INSTALL(TARGETS mll DESTINATION ${CMAKE_INSTALL_PREFIX})

########################### gtest ###########################

ADD_EXECUTABLE(gtest	${GTEST_SRC_FILES}						
						${CORE_TEST_SRC_FILES}
						${CLASSIFIERS_TEST_SRC_FILES}
)
TARGET_LINK_LIBRARIES(gtest mll)
INSTALL(TARGETS gtest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

########################### main ###########################

ADD_EXECUTABLE(main     ${MAIN_SRC_FILES}
						${CLASSIFIERS_HDR_FILES}
						${CLASSIFIERS_SRC_FILES}
)
TARGET_LINK_LIBRARIES(main mll)
INSTALL(TARGETS main RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})


########################### Build documentation with Doxygen ###########################
INCLUDE(${CMAKE_ROOT}/Modules/Documentation.cmake OPTIONAL)
OPTION(BUILD_DOCUMENTATION "Build MachineLearningLibrary reference documentation using doxygen (use: make DoxygenDoc)" OFF)
MARK_AS_ADVANCED(CLEAR BUILD_DOCUMENTATION)

# This processes our doxyfile.cmake and substitutes paths to generate
# a final Doxyfile
CONFIGURE_FILE(${MachineLearningLibrary_SOURCE_DIR}/doc/doxyfile.cmake
	${MachineLearningLibrary_BINARY_DIR}/doxyfile.cmake
)    

# Searching for doxygen location and initializes ${DOXYGEN} when found
FIND_PACKAGE(DOXYGEN)

#INSTALL(DIRECTORY ${MachineLearningLibrary_BINARY_DIR} DESTINATION ${CMAKE_INSTALL_PREFIX})

# This creates a new target to build documentation.
# It runs ${DOXYGEN} which is the full path and executable to
# Doxygen on your system, set by the FindDoxygen.cmake module
# (called by FindDocumentation.cmake).
# It runs the final generated Doxyfile against it.
# The DOT_PATH is substituted into the Doxyfile.
ADD_CUSTOM_TARGET(doc ${DOXYGEN}
	${MachineLearningLibrary_BINARY_DIR}/doxyfile.cmake
)
