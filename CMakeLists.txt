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
    COMMENT "Creating shaders directory in 
output"
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
# Usunięto komendy kopiowania dla glow.vert i glow.frag

add_custom_command(TARGET PetriDish POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SHADER_DIR}/point.vert"
        "${OUTPUT_SHADER_DIR}/point.vert"
    COMMENT "Copying point.vert"
)
add_custom_command(TARGET PetriDish POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SHADER_DIR}/point.frag"
        "${OUTPUT_SHADER_DIR}/point.frag"
    COMMENT "Copying point.frag"
)
add_custom_command(TARGET PetriDish POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SHADER_DIR}/antibiotic.vert"
        "${OUTPUT_SHADER_DIR}/antibiotic.vert"
    COMMENT "Copying antibiotic.vert"
)
add_custom_command(TARGET PetriDish POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SHADER_DIR}/antibiotic.frag"
        "${OUTPUT_SHADER_DIR}/antibiotic.frag"
    COMMENT "Copying antibiotic.frag"
)

add_custom_command(TARGET PetriDish POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SHADER_DIR}/petridish.vert"
        "${OUTPUT_SHADER_DIR}/petridish.vert"
    COMMENT "Copying petridish.vert"
)
add_custom_command(TARGET PetriDish POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SHADER_DIR}/petridish.frag"
        "${OUTPUT_SHADER_DIR}/petridish.frag"
    COMMENT "Copying petridish.frag"
)

set(ASSET_DIR_SRC "${CMAKE_SOURCE_DIR}/src/assets") 
set(MODEL_DIR_SRC "${ASSET_DIR_SRC}/models")
set(TEXTURE_DIR_SRC "${ASSET_DIR_SRC}/textures")

set(OUTPUT_ASSET_DIR "$<TARGET_FILE_DIR:PetriDish>/assets")
set(OUTPUT_MODEL_DIR "${OUTPUT_ASSET_DIR}/models")
set(OUTPUT_TEXTURE_DIR "${OUTPUT_ASSET_DIR}/textures")

add_custom_command(TARGET PetriDish POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_MODEL_DIR}"
    COMMENT "Creating models directory in output"
)
add_custom_command(TARGET PetriDish POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_TEXTURE_DIR}"
    COMMENT "Creating textures directory in output"
)

add_custom_command(TARGET PetriDish POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${MODEL_DIR_SRC}/przykrywka.obj" 
        "${OUTPUT_MODEL_DIR}/przykrywka.obj"
    COMMENT "Copying przykrywka.obj"
)

add_custom_command(TARGET PetriDish POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${MODEL_DIR_SRC}/szalka.obj" 
        "${OUTPUT_MODEL_DIR}/szalka.obj"
    COMMENT "Copying szalka.obj"
)

add_custom_command(TARGET PetriDish POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${MODEL_DIR_SRC}/agar.obj"  
        "${OUTPUT_MODEL_DIR}/agar.obj"
    COMMENT "Copying agar.obj"
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