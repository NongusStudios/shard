#pragma once

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <imgui.h>

#include "gfx/gfx.hpp"

namespace shard{
    namespace imgui{
        void checkResult(VkResult result);
        ImGuiIO& init(
            GLFWwindow* window, gfx::Graphics& gfx, gfx::DescriptorPool& descPool,
            VkSampleCountFlagBits samples
        );
        void terminate();

        void startFrame();
        void endFrame(VkCommandBuffer cmd);
    } // namespace imgui
} // namespace shard

/**
    Copyright 2022 Nongus Studios (https://github.com/NongusStudios/shard)
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/