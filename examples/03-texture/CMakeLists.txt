add_executable(03.out main.cpp)

target_link_libraries(03.out
    dl
    vulkan
    glfw
    shard
)