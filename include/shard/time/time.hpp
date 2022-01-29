#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstdint>

namespace shard{
    struct Time{
        float dt=0.0f;
        float elapsed=0.0f;
        uint32_t ticks=0;
        float fps=0.0f;
    };
    namespace time{
        void updateTime(Time& time);
    } // namespace time
    

}