cmake_minimum_required(VERSION 3.20)
project(shard VERSION 1.0)

add_subdirectory(lib/glfw)

# Compile Engine
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

if(MSVC)
    add_compile_options(/W2)
else()
    add_compile_options(-Wall -pedantic)
endif()

include_directories(
    lib/glfw/include
    lib/glm
    lib/stb
    lib/vma/include
    lib/shardExtraDeps
    lib/imgui
    lib/imgui/backends
    include
    "C:/VulkanSDK/1.3.211.0/Include"
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

    lib/imgui/*.cpp
)
set(SHARD_LIB shard)
add_library(${SHARD_LIB}
    ${SHARD_SRC}
    lib/imgui/backends/imgui_impl_glfw.cpp
    lib/imgui/backends/imgui_impl_vulkan.cpp
)

# Uncomment the example you want to build
#add_subdirectory(examples/01-triangle)
#add_subdirectory(examples/02-cube)
#add_subdirectory(examples/03-texture)
#add_subdirectory(examples/04-model)
#add_subdirectory(examples/05-imgui)
add_subdirectory(examples/06-compute-boids)

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

    shaders/*.comp
    shaders/**/*.comp
    shaders/**/**/*.comp
    shaders/**/**/**/*.comp
    shaders/**/**/**/**/*.comp
    examples/*.comp
    examples/**/*.comp
    examples/**/**/*.comp
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