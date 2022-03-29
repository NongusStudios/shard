add_executable(out main.cpp)

target_link_libraries(out
    dl
    vulkan
    glfw
    shard
)