cmake_minimum_required(VERSION 3.16)
project(PetriDish)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True) 

add_subdirectory(external/glfw)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/external/glfw/include
    ${CMAKE_SOURCE_DIR}/external/glm 
    ${CMAKE_SOURCE_DIR}/external/glew/include
    ${CMAKE_SOURCE_DIR}/external/imgui 
    ${CMAKE_SOURCE_DIR}/external/imgui/backends
)

link_directories(${CMAKE_SOURCE_DIR}/external/glew/lib/Release/Win32)

file(GLOB APP_SOURCES
    "src/*.cpp"
    "src/Simulation/*.cpp"  
    "src/Rendering/*.cpp"
    "src/Utils/*.cpp"
    "src/App/*.cpp"
)

set(IMGUI_SOURCES
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_opengl3.cpp
)

add_executable(PetriDish ${APP_SOURCES} ${IMGUI_SOURCES})

set(SHADER_DIR "${CMAKE_SOURCE_DIR}/src/shaders")

set(OUTPUT_SHADER_DIR "$<TARGET_FILE_DIR:PetriDish>/shaders")

add_custom_command(TARGET PetriDish POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_SHADER_DIR}"
    COMMENT "Creating shaders directory in output"
)
add_custom_command(TARGET PetriDish POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SHADER_DIR}/bacteria.vert"
        "${OUTPUT_SHADER_DIR}/bacteria.vert"
    COMMENT "Copying bacteria.vert"
)
add_custom_command(TARGET PetriDish POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SHADER_DIR}/bacteria.frag"
        "${OUTPUT_SHADER_DIR}/bacteria.frag"
    COMMENT "Copying bacteria.frag"
)

set(GLEW_DLL_PATH "${CMAKE_SOURCE_DIR}/external/glew/bin/Release/Win32/glew32.dll")

if(EXISTS "${GLEW_DLL_PATH}")
    add_custom_command(TARGET PetriDish POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${GLEW_DLL_PATH}"
            $<TARGET_FILE_DIR:PetriDish>
        COMMENT "Copying glew32.dll to output directory"
    )
else()
    message(WARNING "GLEW DLL not found at ${GLEW_DLL_PATH}. Manual copy might be required.")
endif()

target_link_libraries(PetriDish PUBLIC glfw opengl32 glew32)

target_compile_definitions(PetriDish PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLEW)
