#pragma once

#include <optional>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vk_mem_alloc.h>

#include "../def.hpp"
#include "../utils.hpp"

#define SHARD_GFX_VK_API_VERSION VK_API_VERSION_1_2

namespace shard{
    namespace gfx{
        struct SwapchainSupportDetails {
            SwapchainSupportDetails();
            SwapchainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface);

            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

        struct QueueFamilyIndices {
            QueueFamilyIndices();
            QueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface);

            std::optional<uint32_t> graphics;
            std::optional<uint32_t> present;
            bool complete() { return graphics.has_value() && present.has_value(); }
        };

        class Device{
            public:
                Device(GLFWwindow* win);
                ~Device();

                shard_delete_copy_constructors(Device);

                VkInstance instance() { return _instance; }
                VkPhysicalDevice pDevice() { return _pDevice; }
                VkDevice device() { return _device; }
                VkSurfaceKHR surface() { return _surface; }
                VkQueue graphicsQueue() { return _graphicsQueue; }
                VkQueue presentQueue() { return _presentQueue; }
                VkCommandPool commandPool() { return _commandPool; }
                VkPhysicalDeviceProperties properties() {
                    VkPhysicalDeviceProperties p = {};
                    vkGetPhysicalDeviceProperties(_pDevice, &p);
                    return p;
                }
                VmaAllocator allocator() { return _allocator; }
                GLFWwindow* window() { return _window; }

                void waitIdle(){
                    vkDeviceWaitIdle(_device);
                }

                SwapchainSupportDetails getSwapchainSupportDetails(){
                    return SwapchainSupportDetails(_pDevice, _surface);
                }
                QueueFamilyIndices getQueueFamilyIndices(){
                    return QueueFamilyIndices(_pDevice, _surface);
                }
                uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
                VkFormat findSupportedFormat(
                    const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                    VkFormatFeatureFlags features
                );

                void createImageWithInfo(
                    const VkImageCreateInfo &imageInfo,
                    VkMemoryPropertyFlags properties,
                    VkImage &image,
                    VkDeviceMemory &imageMemory
                );

                VkCommandBuffer beginSingleTimeCommands();
                void endSingleTimeCommands(VkCommandBuffer commandBuffer);
                void transitionImageLayout(
                    VkImage image, VkFormat format,
                    uint32_t mipLevels, VkImageAspectFlags aspectMask,
                    VkImageLayout oldLayout, VkImageLayout newLayout
                );
                void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
                void copyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height);
            private:
                void init();
                void cleanup();
                void createInstance();
                void createSurface();
                void pickPhysicalDevice();
                void createLogicalDevice();
                void createAllocator();
                void createCommandPool();

                // Helper
                bool isDeviceSuitable(VkPhysicalDevice device);
                uint32_t rateDevice(VkPhysicalDevice device);
                std::vector<const char *> getRequiredExtensions();
                bool checkValidationLayerSupport();
                bool checkGlfwRequiredExtensionSupport(const std::vector<const char*>& exts);
                bool checkDeviceExtensionSupport(VkPhysicalDevice device);

                VkInstance _instance;
                VkPhysicalDevice _pDevice = VK_NULL_HANDLE;
                GLFWwindow* _window;
                VkCommandPool _commandPool;

                VkDevice _device;
                VkSurfaceKHR _surface;
                VkQueue _graphicsQueue;
                VkQueue _presentQueue;

                VmaAllocator _allocator;

                const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
                const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
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