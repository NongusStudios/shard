#pragma once

#include "device.hpp"

namespace shard{
    namespace gfx{
        class ShaderModule{
            public:
                ShaderModule(Device& _device):
                    device{_device}
                {}
                ShaderModule(Device& _device, const char* filePath);
                ShaderModule(Device& _device, const std::vector<char>& spv);
                ShaderModule(ShaderModule& sm);
                ShaderModule(ShaderModule&& sm);
                ~ShaderModule();

                ShaderModule& operator = (ShaderModule& sm){
                    assert(&device == &sm.device);
                    vkDestroyShaderModule(device.device(), _shaderModule, nullptr);
                    _shaderModule = sm._shaderModule;
                    sm._shaderModule = VK_NULL_HANDLE;
                    return *this;
                }
                ShaderModule& operator = (ShaderModule&& sm){
                    assert(&device == &sm.device);
                    vkDestroyShaderModule(device.device(), _shaderModule, nullptr);
                    _shaderModule = sm._shaderModule;
                    sm._shaderModule = VK_NULL_HANDLE;
                    return *this;
                }

                bool valid() const { return _shaderModule != VK_NULL_HANDLE; }

                VkShaderModule shaderModule() { return _shaderModule; }
                const VkShaderModule shaderModule() const { return _shaderModule; }
            private:
                void init(const std::vector<char>& spv);
                
                Device& device;
                VkShaderModule _shaderModule=VK_NULL_HANDLE;
        };
    } // namespace gfx
} // namespace shard
