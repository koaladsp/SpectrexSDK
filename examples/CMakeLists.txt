cmake_minimum_required(VERSION 3.18)

# ExternalProject cmake dependency
include(ExternalProject)

# macOS specific project settings
if(APPLE)
    # Set minimum required OSX deployment version
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version")
    # Intel and M1 architectures
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for Mac OS X" FORCE)
endif()

project(ExampleApp
    LANGUAGES CXX C
    VERSION   1.0.0
)

# C++ Standard
set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Statically link runtimes on Windows (either MultiThreaded or MultiThreadedDebug depending on configuration)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(MSVC_RUNTIME "MT")

### JUCE dependency
add_subdirectory(3rd/JUCE)

###### Spectrex BEGIN

### Third party dependencies
# glad
set(GLAD_PROFILE    "core"   CACHE STRING "" FORCE)
set(GLAD_API        "gl=4.1" CACHE STRING "" FORCE)
set(GLAD_GENERATOR  "c"      CACHE STRING "" FORCE)
set(GLAD_EXTENSIONS ""       CACHE STRING "" FORCE)
set(GLAD_SPEC       "gl"     CACHE STRING "" FORCE)
add_subdirectory(3rd/glad)

# GLM
add_subdirectory(3rd/glm)

# GSL
add_subdirectory(3rd/GSL)

# ipp
if(WIN32)
    add_subdirectory(3rd/ipp/2021.1.1-windows)
elseif(APPLE)
    # It so happens that one of the BLAS providers on MacOS, supported by CMake's FindBLAS is the Accelerate
    # framework, which also provides VDSP
    set(BLAS_VENDOR Apple)
    find_package(BLAS REQUIRED)
endif()

### Spectrex package 
set(SPECTREX_PATH ${PROJECT_SOURCE_DIR}/..) # Should point to the root directory of the spectrex SDK
if(MSVC)
    if(NOT CMAKE_VS_PLATFORM_NAME)
        message(FATAL_ERROR "Please use -A Win32 or -A x64 to specify whether you're building for 32-bit or 64-bit!")
    endif()
    include(${SPECTREX_PATH}/cmake/msvc/v${MSVC_TOOLSET_VERSION}/${MSVC_RUNTIME}/${CMAKE_VS_PLATFORM_NAME}/Spectrex.cmake)
elseif(APPLE)
    include(${SPECTREX_PATH}/cmake/macos/Spectrex.cmake)
endif()

# For project importing this library: Debug configuration uses Debug, all others use Release
set_target_properties(Spectrex::Spectrex PROPERTIES
    MAP_IMPORTED_CONFIG_DEBUG Debug
    MAP_IMPORTED_CONFIG_RELEASE Release
    MAP_IMPORTED_CONFIG_MINSIZEREL Release
    MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
)
add_library(Spectrex ALIAS Spectrex::Spectrex)

###### Spectrex END

if(MINGW)
    # Enable static libc++ linking if using llvm-mingw
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -Wl,-pdb=")
endif()

# Examples
add_subdirectory(Viz2DApp)
add_subdirectory(Viz3DApp)
