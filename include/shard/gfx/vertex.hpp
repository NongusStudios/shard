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
#include "color.hpp"

namespace shard{
    namespace gfx{
        struct Vertex2D{
            glm::vec2 pos;
            glm::vec2 uv;
            Color     color;

            Vertex2D():
                pos{0.0f},
                uv{0.0f},
                color{}
            {}
            Vertex2D(
                const glm::vec2& _pos,
                const glm::vec2& _uv,
                const Color& _color
            ):
                pos{_pos},
                uv{_uv},
                color{_color}
            {}

            bool operator == (const Vertex2D& v) const {
                return pos   == v.pos &&
                       uv    == v.uv  &&
                       color == v.color;
            }

            static VkVertexInputBindingDescription bindingDesc(){
                VkVertexInputBindingDescription binding = {};
                binding.binding = 0;
                binding.stride = sizeof(Vertex2D);
                binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                return binding;
            }

            static std::vector<VkVertexInputAttributeDescription> attributeDescs(){
                std::vector<VkVertexInputAttributeDescription> attrs(3);

                attrs[0].binding = 0;
                attrs[0].location = 0;
                attrs[0].format = VK_FORMAT_R32G32_SFLOAT;
                attrs[0].offset = offsetof(Vertex2D, pos);
                
                attrs[1].binding = 0;
                attrs[1].location = 1;
                attrs[1].format = VK_FORMAT_R32G32_SFLOAT;
                attrs[1].offset = offsetof(Vertex2D, uv);

                attrs[2].binding = 0;
                attrs[2].location = 2;
                attrs[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
                attrs[2].offset = offsetof(Vertex2D, color);

                return attrs;
            }
        };
        struct Vertex3D{
            glm::vec3 pos;
            glm::vec2 uv;
            glm::vec3 normal;
            Color     color;

            Vertex3D():
                pos{0.0f},
                uv{0.0f},
                normal{0.0f},
                color{}
            {}
            Vertex3D(
                const glm::vec3& _pos,
                const glm::vec2& _uv,
                const glm::vec3& _normal,
                const Color& _color
            ):
                pos{_pos},
                uv{_uv},
                normal{_normal},
                color{_color}
            {}

            bool operator == (const Vertex3D& v) const {
                return pos    == v.pos    &&
                       uv     == v.uv     &&
                       normal == v.normal &&
                       color  == v.color;
            }

            static VkVertexInputBindingDescription bindingDesc(){
                VkVertexInputBindingDescription binding = {};
                binding.binding = 0;
                binding.stride = sizeof(Vertex3D);
                binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                return binding;
            }

            static std::vector<VkVertexInputAttributeDescription> attributeDescs(){
                std::vector<VkVertexInputAttributeDescription> attrs(4);

                attrs[0].binding = 0;
                attrs[0].location = 0;
                attrs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
                attrs[0].offset = offsetof(Vertex3D, pos);
                
                attrs[1].binding = 0;
                attrs[1].location = 1;
                attrs[1].format = VK_FORMAT_R32G32_SFLOAT;
                attrs[1].offset = offsetof(Vertex3D, uv);

                attrs[2].binding = 0;
                attrs[2].location = 2;
                attrs[2].format = VK_FORMAT_R32G32B32_SFLOAT;
                attrs[2].offset = offsetof(Vertex3D, normal);

                attrs[3].binding = 0;
                attrs[3].location = 3;
                attrs[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
                attrs[3].offset = offsetof(Vertex3D, color);

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
