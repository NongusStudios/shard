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

            for(const auto& device : devices){
                if(isDeviceSuitable(device)){
                    _pDevice = device;
                    break;
                }
            }

            shard_abort_ifnot(_pDevice != VK_NULL_HANDLE && "No suitable physical devices found!");

            if(shard::IS_DEBUG){
                VkPhysicalDeviceProperties props;
                vkGetPhysicalDeviceProperties(_pDevice, &props);
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
    } // namespace gfx
} // namespace shard