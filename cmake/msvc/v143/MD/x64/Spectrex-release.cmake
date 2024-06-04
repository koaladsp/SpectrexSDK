#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Spectrex::Spectrex" for configuration "Release"
set_property(TARGET Spectrex::Spectrex APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(Spectrex::Spectrex PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/msvc/v143/MD/x64/Release/Spectrex.lib"
  )

list(APPEND _cmake_import_check_targets Spectrex::Spectrex )
list(APPEND _cmake_import_check_files_for_Spectrex::Spectrex "${_IMPORT_PREFIX}/lib/msvc/v143/MD/x64/Release/Spectrex.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
