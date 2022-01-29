#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

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
