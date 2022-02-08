#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>

#include "../utils.hpp"
#include "../def.hpp"

namespace shard{
    namespace gfx{
        struct Vertex{
            glm::vec3 pos;
            glm::vec3 color;

            static VkVertexInputBindingDescription bindingDesc(){
                VkVertexInputBindingDescription binding = {};
                binding.binding = 0;
                binding.stride = sizeof(Vertex);
                binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                return binding;
            }

            static std::vector<VkVertexInputAttributeDescription> attributeDescs(){
                std::vector<VkVertexInputAttributeDescription> attrs(2);

                attrs[0].binding = 0;
                attrs[0].location = 0;
                attrs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
                attrs[0].offset = offsetof(Vertex, pos);
                
                attrs[1].binding = 0;
                attrs[1].location = 1;
                attrs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
                attrs[1].offset = offsetof(Vertex, color);

                return attrs;
            }
        };
    } // namespace gfx
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
