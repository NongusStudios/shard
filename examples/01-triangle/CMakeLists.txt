add_executable(01.out main.cpp)

target_link_libraries(01.out
    dl
    vulkan
    glfw
    shard
)