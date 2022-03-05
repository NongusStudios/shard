#pragma once

#include <memory>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "device.hpp"
#include "swapchain.hpp"
#include "pipeline.hpp"
#include "vertex.hpp"
#include "buffer.hpp"
#include "descriptor.hpp"
#include "image.hpp"
#include "color.hpp"

// Thanks to Brendan Galea for the free init code and for getting me started on vulkan
// (https://github.com/blurrypiano/littleVulkanEngine)
namespace shard{
    namespace gfx{
        class Graphics{
            public:
                Graphics(GLFWwindow* win, bool vsync);
                ~Graphics();

                shard_delete_copy_constructors(Graphics);

                GLFWwindow* window() { return _window; }

                Device& device() { return *_device; };
                Swapchain& swapchain() { return *_swapchain; }
                VkPipelineLayout emptyPipelineLayout() { return _emptyPipelineLayout; }
                PipelineConfigInfo& deafultPipelineConfig() {
                    return _defaultPipelineConfig;
                }
                VkCommandPool commandPool() { return _device->commandPool(); }

                VkCommandBuffer currentCommandBuffer(){
                    assert(
                        isFrameStarted &&
                        "Cannot get command buffer when a frame is not in progress!"
                    );
                    return commandBuffers[currentFrameIndex];
                }
                uint32_t frameIndex(){
                    assert(
                        isFrameStarted &&
                        "Cannot get frame index when a frame is not in progress!"
                    );
                    return currentFrameIndex;
                }

                Pipeline createPipeline(
                    VkPipelineLayout layout,
                    const char* vertFile,
                    const char* fragFile,
                    const std::vector<VkVertexInputBindingDescription>& bindingDescs,
                    const std::vector<VkVertexInputAttributeDescription>& attrDescs,
                    PipelineConfigInfo& config
                );
                Pipeline createPipeline(
                    VkPipelineLayout layout,
                    const std::vector<char>& vertFile,
                    const std::vector<char>& fragFile,
                    const std::vector<VkVertexInputBindingDescription>& bindingDescs,
                    const std::vector<VkVertexInputAttributeDescription>& attrDescs,
                    PipelineConfigInfo& config
                );
                Buffer createVertexBuffer(size_t size, const void* data);
                Buffer createIndexBuffer(size_t size, const void* data);
                Buffer createUniformBuffer(size_t size, const void* data);
                Image createTexture(const char* filePath);
                // Pixel format must be in VK_FORMAT_R8G8B8A8_BIT
                Image createTexture(uint32_t w, uint32_t h, const void* pixels);
                Image createImage(
                    uint32_t w, uint32_t h, uint32_t mipLevels,
                    VkFormat __format, VkImageTiling tiling,
                    VkSampleCountFlagBits samples,
                    VkImageUsageFlags usage,
                    VkImageCreateFlags flags,
                    VmaMemoryUsage memUsage,
                    VkImageAspectFlags aspectMask
                );
                Framebuffer createFramebuffer(std::vector<Image>& attachments);
                Framebuffer createFramebuffer(
                    VkRenderPass renderPass, std::vector<Image>& attachments
                );
                Framebuffer createFramebuffer(std::vector<Image>&& attachments);
                Framebuffer createFramebuffer(
                    VkRenderPass renderPass, std::vector<Image>&& attachments
                );

                VkCommandBuffer beginRenderPass(const Color& clearColor);
                void endRenderPass();
            private:
                void recreateSwapchain();
                void createCommandBuffers();
                void createEmptyPipelineLayout();

                void destroyCommandBuffers();

                const bool VSYNC;
                GLFWwindow* _window;
                PipelineConfigInfo _defaultPipelineConfig;
                std::unique_ptr<Device> _device;
                std::unique_ptr<Swapchain> _swapchain;

                bool isFrameStarted = false;
                uint32_t currentFrameIndex = 0;
                uint32_t imageIndex = 0;
                std::vector<VkCommandBuffer> commandBuffers;
                VkPipelineLayout _emptyPipelineLayout;
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