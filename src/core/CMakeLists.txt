SET(_MLL_CORE_SOURCES
    fix_alloc.cpp
    util.cpp
    dataset.cpp
    dataset_wrapper.cpp
    metadata.cpp
    metadata_wrapper.cpp
    tester.cpp
    cross_validation.cpp
)

FOREACH(_FILENAME ${_MLL_CORE_SOURCES})
    GET_FILENAME_COMPONENT(_FILENAME ${_FILENAME} REALPATH)
    SET(_MLL_CORE_SOURCES_P ${_MLL_CORE_SOURCES_P} ${_FILENAME})
ENDFOREACH()

SET(MLL_CORE_SOURCES ${_MLL_CORE_SOURCES_P} PARENT_SCOPE)
