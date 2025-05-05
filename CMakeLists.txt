cmake_minimum_required(VERSION 3.16)
project(PetriDish)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(external/glfw)
add_subdirectory(external/glm)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Simulation
    ${CMAKE_SOURCE_DIR}/src/Rendering
    ${CMAKE_SOURCE_DIR}/src/Utils
    ${CMAKE_SOURCE_DIR}/src/App
    ${CMAKE_SOURCE_DIR}/external/glfw/include
    ${CMAKE_SOURCE_DIR}/external/glm
    ${CMAKE_SOURCE_DIR}/external/glew/include
)

link_directories(external/glew/lib/Release/Win32) 

file(GLOB SOURCES "src/*.cpp")

add_executable(PetriDish ${SOURCES})

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

