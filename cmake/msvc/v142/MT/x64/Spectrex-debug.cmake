#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Spectrex::Spectrex" for configuration "Debug"
set_property(TARGET Spectrex::Spectrex APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(Spectrex::Spectrex PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/msvc/v142/MT/x64/Debug/Spectrex.lib"
  )

list(APPEND _cmake_import_check_targets Spectrex::Spectrex )
list(APPEND _cmake_import_check_files_for_Spectrex::Spectrex "${_IMPORT_PREFIX}/lib/msvc/v142/MT/x64/Debug/Spectrex.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
