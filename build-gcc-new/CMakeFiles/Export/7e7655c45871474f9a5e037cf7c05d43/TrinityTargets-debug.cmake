#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Trinity::trinity" for configuration "Debug"
set_property(TARGET Trinity::trinity APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(Trinity::trinity PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libtrinity.a"
  )

list(APPEND _cmake_import_check_targets Trinity::trinity )
list(APPEND _cmake_import_check_files_for_Trinity::trinity "${_IMPORT_PREFIX}/lib/libtrinity.a" )

# Import target "Trinity::genesis" for configuration "Debug"
set_property(TARGET Trinity::genesis APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(Trinity::genesis PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libgenesis.a"
  )

list(APPEND _cmake_import_check_targets Trinity::genesis )
list(APPEND _cmake_import_check_files_for_Trinity::genesis "${_IMPORT_PREFIX}/lib/libgenesis.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
