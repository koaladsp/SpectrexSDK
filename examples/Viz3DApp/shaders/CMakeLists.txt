find_package(Python COMPONENTS Interpreter REQUIRED)

add_library(Viz3DShaders INTERFACE)

target_include_directories(Viz3DShaders
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(GenerateShadersHeader
        COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/ToHeader.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        BYPRODUCTS ${CMAKE_CURRENT_LIST_DIR}/Shaders.h
        COMMENT "Generating shaders header"
)

add_dependencies(Viz3DShaders GenerateShadersHeader)
