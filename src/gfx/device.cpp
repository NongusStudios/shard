#include <shard/gfx/device.hpp>

#include <cstring>
#include <set>

namespace shard{
    namespace gfx{
        QueueFamilyIndices::QueueFamilyIndices(){}
        QueueFamilyIndices::QueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface){
            assert(device != VK_NULL_HANDLE);
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            int i = 0;
            for (const auto &queueFamily : queueFamilies){
                if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT){
                    graphics = i;
                }
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
                if (queueFamily.queueCount > 0 && presentSupport)
                    present = i;
                
                if (complete()) break;

                i++;
            }
        }

        SwapchainSupportDetails::SwapchainSupportDetails():
            capabilities{}
        {}
        SwapchainSupportDetails::SwapchainSupportDetails(
            VkPhysicalDevice device, VkSurfaceKHR surface
        ){
            assert(device != VK_NULL_HANDLE);
            assert(surface != VK_NULL_HANDLE);

            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);

            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

            if (formatCount != 0) {
                formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(
                    device, surface, &formatCount, formats.data()
                );
            }

            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

            if (presentModeCount != 0) {
                presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(
                    device,
                    surface,
                    &presentModeCount,
                    presentModes.data()
                );
            }
        }

        Device::Device(GLFWwindow* win):
            _window{win}
        {
            assert(_window != nullptr);
            init();
        }
        Device::~Device(){
            cleanup();
        }

        void Device::init(){
            createInstance();
            createSurface();
            pickPhysicalDevice();
            createLogicalDevice();
            createAllocator();
            createCommandPool();
        }
        void Device::cleanup(){
            vkDestroyCommandPool(_device, _commandPool, nullptr);
            vmaDestroyAllocator(_allocator);
            vkDestroyDevice(_device, nullptr);
            vkDestroySurfaceKHR(_instance, _surface, nullptr);
            vkDestroyInstance(_instance, nullptr);
        }
        void Device::createInstance(){
            if(shard::IS_DEBUG){
                shard_abort_ifnot(checkValidationLayerSupport());
            }

            VkApplicationInfo appInfo = {};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "Shard App";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "Shard Engine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = SHARD_GFX_VK_API_VERSION;

            VkInstanceCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;

            auto extensions = getRequiredExtensions();
            shard_abort_ifnot(checkGlfwRequiredExtensionSupport(extensions));
            createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
            createInfo.ppEnabledExtensionNames = extensions.data();

            if(shard::IS_DEBUG){
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
            }

            shard_abort_ifnot(
                vkCreateInstance(&createInfo, nullptr, &_instance) == VK_SUCCESS
            );
        }
        void Device::createSurface(){
            shard_abort_ifnot(
                glfwCreateWindowSurface(
                    _instance,
                    _window,
                    nullptr,
                    &_surface
                ) == VK_SUCCESS
            );
        }
        void Device::pickPhysicalDevice(){
            uint32_t deviceCount = 0;
            vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
            shard_abort_ifnot(deviceCount != 0 && "No devices found!");
            std::vector<VkPhysicalDevice> devices(deviceCount);
            vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

            uint32_t currentHighScore = 0;
            uint32_t highestDeviceIndex = 0;
            uint32_t i = 0;
            for(const auto& device : devices){
                uint32_t score = rateDevice(device);
                if(score > currentHighScore){
                    currentHighScore = score;
                    highestDeviceIndex = i;
                }
                i++;
            }
            shard_abort_ifnot(currentHighScore > 0 && "No suitable physical device found!");
            _pDevice = devices[highestDeviceIndex];

            shard_abort_ifnot(_pDevice != VK_NULL_HANDLE && "No suitable physical devices found!");

            if(shard::IS_DEBUG){
                VkPhysicalDeviceProperties props = properties();
                std::cout << "_pDevice: " << props.deviceName << "\n";
            }
        }
        void Device::createLogicalDevice(){
            QueueFamilyIndices indices = getQueueFamilyIndices();

            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
            std::set<uint32_t> uniqueQueueFamilies = {
                indices.graphics.value(), indices.present.value()
            };

            float queuePriority = 1.0f;
            for(uint32_t queueFamily : uniqueQueueFamilies){
                VkDeviceQueueCreateInfo queueCreateInfo = {};
                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex = queueFamily;
                queueCreateInfo.queueCount = 1;
                queueCreateInfo.pQueuePriorities = &queuePriority;
                queueCreateInfos.push_back(queueCreateInfo);
            }

            VkPhysicalDeviceFeatures deviceFeatures = {};
            deviceFeatures.samplerAnisotropy = VK_TRUE;
            deviceFeatures.fillModeNonSolid = VK_TRUE;

            VkDeviceCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

            createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
            createInfo.pQueueCreateInfos = queueCreateInfos.data();

            createInfo.pEnabledFeatures = &deviceFeatures;
            createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
            createInfo.ppEnabledExtensionNames = deviceExtensions.data();

            // This is a deprecated feature, don't care.
            if(shard::IS_DEBUG){
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
            }

            shard_abort_ifnot(vkCreateDevice(_pDevice, &createInfo, nullptr, &_device) == VK_SUCCESS);

            vkGetDeviceQueue(_device, indices.graphics.value(), 0, &_graphicsQueue);
            vkGetDeviceQueue(_device, indices.present.value(), 0, &_presentQueue);
        }
        void Device::createAllocator(){
            VmaAllocatorCreateInfo allocInfo = {};
            allocInfo.vulkanApiVersion = SHARD_GFX_VK_API_VERSION;
            allocInfo.physicalDevice = _pDevice;
            allocInfo.device = _device;
            allocInfo.instance = _instance;

            shard_abort_ifnot(vmaCreateAllocator(&allocInfo, &_allocator) == VK_SUCCESS);
        }
        void Device::createCommandPool(){
            QueueFamilyIndices indices = getQueueFamilyIndices();

            VkCommandPoolCreateInfo poolInfo = {};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.queueFamilyIndex = indices.graphics.value();
            poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
                             VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            shard_abort_ifnot(
                vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool) == VK_SUCCESS
            );
        }

        bool Device::isDeviceSuitable(VkPhysicalDevice device){
            QueueFamilyIndices indices(device, _surface);

            bool extensionsSupported = checkDeviceExtensionSupport(device);

            bool swapChainAdequate = false;
            if(extensionsSupported){
                SwapchainSupportDetails swapChainSupport(device, _surface);
                swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
            }

            VkPhysicalDeviceFeatures supportedFeatures;
            vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

            return indices.complete() && extensionsSupported && swapChainAdequate &&
                    supportedFeatures.samplerAnisotropy;
        }
        uint32_t Device::rateDevice(VkPhysicalDevice device){
            if(!isDeviceSuitable(device)) return 0;

            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(device, &props);

            uint32_t score = 1;
            if(props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
                score += 1000;
            }
            score += props.limits.maxImageDimension2D;

            return score;
        }
        std::vector<const char*> Device::getRequiredExtensions(){
            uint32_t glfwExtensionCount = 0;
            const char **glfwExtensions;
            glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

            return extensions;
        }
        bool Device::checkValidationLayerSupport(){
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

            for(const char *layerName : validationLayers){
                bool layerFound = false;

                for(const auto &layerProperties : availableLayers){
                    if(strcmp(layerName, layerProperties.layerName) == 0){
                        layerFound = true;
                        break;
                    }
                }

                if(!layerFound){
                    return false;
                }
            }

            return true;
        }
        bool Device::checkGlfwRequiredExtensionSupport(const std::vector<const char*>& exts){
            uint32_t extCount = 0;
            vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
            std::vector<VkExtensionProperties> extProperties(extCount);
            vkEnumerateInstanceExtensionProperties(nullptr, &extCount, extProperties.data());

            for(const auto& ext : exts){
                bool supported = false;
                for(const auto& extProp : extProperties){
                    if(strcmp(ext, extProp.extensionName) == 0)
                        supported = true;
                }
                if(!supported) return false;
            }
            return true;
        }
        bool Device::checkDeviceExtensionSupport(VkPhysicalDevice device){
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(
                device,
                nullptr,
                &extensionCount,
                availableExtensions.data()
            );

            std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

            for (const auto &extension : availableExtensions) {
                requiredExtensions.erase(extension.extensionName);
            }

            return requiredExtensions.empty();
        }

        uint32_t Device::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties){
            VkPhysicalDeviceMemoryProperties memProperties;
            vkGetPhysicalDeviceMemoryProperties(_pDevice, &memProperties);
            for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++){
                if(
                    (typeFilter & (1 << i)) && 
                    (memProperties.memoryTypes[i].propertyFlags & properties) == properties
                  ){
                    return i;
                }
            }

            std::cerr << SHARD_FUNC << "(): Failed to find suitable memory type!\n";
            std::abort();
        }
        VkFormat Device::findSupportedFormat(
            const std::vector<VkFormat> &candidates, VkImageTiling tiling,
            VkFormatFeatureFlags features
        ){
            for(VkFormat format : candidates){
                VkFormatProperties props;
                vkGetPhysicalDeviceFormatProperties(_pDevice, format, &props);

                if(tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                    return format;
                } else if(
                    tiling == VK_IMAGE_TILING_OPTIMAL &&
                    (props.optimalTilingFeatures & features) == features
                ){
                    return format;
                }
            }

            std::cerr << SHARD_FUNC << "(): Failed to find supported format!\n";
            std::abort();
        }
        void Device::createImageWithInfo(
            const VkImageCreateInfo &imageInfo,
            VkMemoryPropertyFlags properties,
            VkImage &image,
            VkDeviceMemory &imageMemory
        ){
            shard_abort_ifnot(
                vkCreateImage(_device, &imageInfo, nullptr, &image) == VK_SUCCESS
            );

            VkMemoryRequirements memRequirements;
            vkGetImageMemoryRequirements(_device, image, &memRequirements);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
            
            shard_abort_ifnot(
                vkAllocateMemory(_device, &allocInfo, nullptr, &imageMemory) == VK_SUCCESS
            );
            shard_abort_ifnot(
                vkBindImageMemory(_device, image, imageMemory, 0) == VK_SUCCESS
            );
        }

        VkCommandBuffer Device::beginSingleTimeCommands(){
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = _commandPool;
            allocInfo.commandBufferCount = 1;

            VkCommandBuffer commandBuffer;
            vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer);

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(commandBuffer, &beginInfo);
            return commandBuffer;
        }
        void Device::endSingleTimeCommands(VkCommandBuffer commandBuffer){
            vkEndCommandBuffer(commandBuffer);

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(_graphicsQueue);

            vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);
        }
        void Device::transitionImageLayout(
            VkImage image, VkFormat format,
            uint32_t mipLevels, VkImageAspectFlags aspectMask,
            VkImageLayout oldLayout, VkImageLayout newLayout
        ){
            auto commandBuffer = beginSingleTimeCommands();

            VkImageMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = oldLayout;
            barrier.newLayout = newLayout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = image;
            barrier.subresourceRange.aspectMask = aspectMask;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = mipLevels;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            VkPipelineStageFlags srcStage;
            VkPipelineStageFlags dstStage;
            if(
                oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
                newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            ){
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            } else if(
                oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            ){
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            } else {
                shard_log_and_abort("unsupported layout transition!");
            }

            vkCmdPipelineBarrier(
                commandBuffer,
                srcStage, dstStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );

            endSingleTimeCommands(commandBuffer);
        }
        void Device::copyBuffer(
            VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size
        ){
            VkCommandBuffer commandBuffer = beginSingleTimeCommands();

            VkBufferCopy copyRegion{};
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = 0;
            copyRegion.size = size;
            vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

            endSingleTimeCommands(commandBuffer);
        }
        void Device::copyBufferToImage(
            VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height
        ){
            auto singleCmdBuf = beginSingleTimeCommands();

            VkBufferImageCopy region = {};
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;

            region.imageOffset = {0, 0, 0};
            region.imageExtent = {
                width,
                height,
                1
            };

            vkCmdCopyBufferToImage(
                singleCmdBuf,
                srcBuffer,
                dstImage,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &region
            );

            endSingleTimeCommands(singleCmdBuf);
        }
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