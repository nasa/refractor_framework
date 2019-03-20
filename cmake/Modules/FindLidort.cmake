# Locate Lidort library


FIND_LIBRARY(_LIDORT_LIBRARY
  NAMES lidort
  HINTS $ENV{LIDORT_DIR} ${LIDORT_DIR}
  PATH_SUFFIXES lib64 lib
)

IF(_LIDORT_LIBRARY)
    SET(LIDORT_LIBRARIES
        "${_LIDORT_LIBRARY}" CACHE STRING "Lidort Libraries")
ENDIF(_LIDORT_LIBRARY)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Lidort DEFAULT_MSG LIDORT_LIBRARIES)
