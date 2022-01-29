#pragma once

#include "device.hpp"
#include "../utils.hpp"

#include <memory>

namespace shard{
    namespace gfx{
        class Swapchain{
            public:
                static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;
                static constexpr bool VSYNC = false;

                Swapchain(Device& refDevice, VkExtent2D winExtent);
                Swapchain(Device& refDevice, VkExtent2D winExtent, std::shared_ptr<Swapchain> previous);
                ~Swapchain();

                shard_delete_copy_constructors(Swapchain);
                
                VkFramebuffer getFrameBuffer(uint32_t index) { return swapchainFramebuffers[index]; }
                VkRenderPass renderPass() { return _renderPass; }
                VkImageView getImageView(uint32_t index) { return swapchainImageViews[index]; }
                size_t imageCount() { return swapchainImages.size(); }
                VkFormat swapchainImageFormat() { return _swapchainImageFormat; }
                VkExtent2D swapchainExtent() { return _swapchainExtent; }
                uint32_t width() { return _swapchainExtent.width; }
                uint32_t height() { return _swapchainExtent.height; }

                float extentAspectRatio() {
                    return static_cast<float>(_swapchainExtent.width) /
                           static_cast<float>(_swapchainExtent.height);
                }
                VkFormat findDepthFormat();

                VkResult acquireNextImage(uint32_t *imageIndex);
                VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);
            private:
                void init();
                void createSwapchain();
                void createImageViews();
                void createRenderPass();
                void createDepthResources();
                void createFramebuffers();
                void createSyncObjects();

                VkSurfaceFormatKHR chooseSwapSurfaceFormat(
                    const std::vector<VkSurfaceFormatKHR> &availableFormats
                );
                VkPresentModeKHR chooseSwapPresentMode(
                    const std::vector<VkPresentModeKHR> &availablePresentModes
                );
                VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

                VkFormat _swapchainImageFormat;
                VkFormat swapchainDepthFormat;
                VkExtent2D _swapchainExtent;

                std::vector<VkFramebuffer> swapchainFramebuffers;
                VkRenderPass _renderPass;

                std::vector<VkImage> depthImages;
                std::vector<VkDeviceMemory> depthImageMemorys;
                std::vector<VkImageView> depthImageViews;
                std::vector<VkImage> swapchainImages;
                std::vector<VkImageView> swapchainImageViews;

                VkExtent2D windowExtent;

                VkSwapchainKHR swapchain;
                std::shared_ptr<Swapchain> oldSwapchain;

                std::vector<VkSemaphore> imageAvailableSemaphores;
                std::vector<VkSemaphore> renderFinishedSemaphores;
                std::vector<VkFence> inFlightFences;
                std::vector<VkFence> imagesInFlight;
                size_t currentFrame = 0;

                Device& device;
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