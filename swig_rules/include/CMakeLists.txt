# Add SWIG files include in build
list(APPEND SWIG_INTERFACE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/swig_std.i
    ${CMAKE_CURRENT_SOURCE_DIR}/swig_array.i
    ${CMAKE_CURRENT_SOURCE_DIR}/swig_boost_array.i
    ${CMAKE_CURRENT_SOURCE_DIR}/swig_rational.i)
file(GLOB SWIG_INCLUDE_FILES_ADD ${CMAKE_CURRENT_SOURCE_DIR}/*.i)  
list(APPEND SWIG_INCLUDE_FILES ${SWIG_INCLUDE_FILES_ADD})

list(APPEND SWIG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Expose to the parent scope
set(SWIG_INTERFACE_FILES ${SWIG_INTERFACE_FILES} PARENT_SCOPE)
set(SWIG_INCLUDE_FILES ${SWIG_INCLUDE_FILES} PARENT_SCOPE)
set(SWIG_INCLUDE_DIR ${SWIG_INCLUDE_DIR} PARENT_SCOPE)
