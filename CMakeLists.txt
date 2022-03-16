cmake_minimum_required(VERSION 3.20)
project(shard VERSION 1.0)

add_subdirectory(lib/glfw)

# Compile Engine
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -pedantic)
endif()

set(GLFW_LIB ${CMAKE_SOURCE_DIR}/build/lib/glfw/src/libglfw3.a)

include_directories(
    lib/glfw/include
    lib/glm
    lib/stb
    lib/vma/include
    lib
    include
)
file(GLOB SHARD_SRC
    include/shard/*.hpp
    include/shard/**/*.hpp
    include/shard/**/**/*.hpp
    include/shard/**/**/**/*.hpp
    src/*.cpp
    src/**/*.cpp
    src/**/**/*.cpp
    src/**/**/**/*.cpp
)
set(SHARD_LIB shard)
add_library(${SHARD_LIB}
    ${SHARD_SRC}
)

set(MODERN_ART examples/modern_art.cpp)
set(MONKEY examples/monkey.cpp)
set(BASIC_RENDERING examples/basic_rendering.cpp)
set(PONG examples/pong.cpp)
set(CURRENT_EXAMPLE ${BASIC_RENDERING})

set(OUT out)
add_executable(${OUT} ${CURRENT_EXAMPLE})

target_link_libraries(${OUT}
    dl
    vulkan
    ${GLFW_LIB}
    ${SHARD_LIB}
)

# Compile Shaders
file(GLOB GLSL_FILES 
    shaders/*.vert
    shaders/**/*.vert
    shaders/**/**/*.vert
    shaders/**/**/**/*.vert
    shaders/**/**/**/**/*.vert
    shaders/*.frag
    shaders/**/*.frag
    shaders/**/**/*.frag
    shaders/**/**/**/*.frag
    shaders/**/**/**/**/*.frag

    examples/*.vert
    examples/**/*.vert
    examples/**/**/*.vert
    examples/*.frag
    examples/**/*.frag
    examples/**/**/*.frag
)

foreach(GLSL_FILE ${GLSL_FILES})
    get_filename_component(FILE_NAME ${GLSL_FILE} NAME)
    get_filename_component(FILE_PATH ${GLSL_FILE} DIRECTORY)
    set(SPIRV "${FILE_PATH}/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND glslc -o ${SPIRV} ${GLSL_FILE}
        DEPENDS ${GLSL_FILE}
    )
    list(APPEND SPIRV_FILES ${SPIRV})
endforeach(GLSL_FILE)

add_custom_target(shaders ALL
    DEPENDS ${SPIRV_FILES}
)