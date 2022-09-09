#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Tracy::TracyClient" for configuration "Debug"
set_property(TARGET Tracy::TracyClient APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(Tracy::TracyClient PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libTracyClientd.so"
  IMPORTED_SONAME_DEBUG "libTracyClientd.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS Tracy::TracyClient )
list(APPEND _IMPORT_CHECK_FILES_FOR_Tracy::TracyClient "${_IMPORT_PREFIX}/lib/libTracyClientd.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
