add_executable(02.out main.cpp)

target_link_libraries(02.out
    dl
    vulkan
    glfw
    shard
)