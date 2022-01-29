#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <memory>

#include "../def.hpp"
#include "../utils.hpp"

#include "device.hpp"
#include "swapchain.hpp"

namespace shard{
    namespace gfx{
        struct PipelineConfigInfo{
            PipelineConfigInfo(){};

            void makeDefault();
            void makeWireframe();

            PipelineConfigInfo(const PipelineConfigInfo&) = delete;
            PipelineConfigInfo& operator = (const PipelineConfigInfo&) = delete;

            VkPipelineViewportStateCreateInfo viewportInfo = {};
            VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
            VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};
            VkPipelineMultisampleStateCreateInfo multisampleInfo = {};
            VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
            VkPipelineColorBlendStateCreateInfo colorBlendInfo = {};
            VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {};
            std::vector<VkDynamicState> dynamicStateEnables = {};
            VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
            uint32_t subpass = 0;
        };

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

                bool valid() { return _shaderModule != VK_NULL_HANDLE; }

                VkShaderModule shaderModule() { return _shaderModule; }
            private:
                void init(const std::vector<char>& spv);

                VkShaderModule _shaderModule;
                Device& device;
        };

        class Pipeline{
            public:
                Pipeline(Device& _device):
                    device{_device},
                    _pipeline{VK_NULL_HANDLE}
                {}
                Pipeline(
                    Device& _device,
                    Swapchain& _swapchain,
                    VkPipelineLayout layout,
                    const std::vector<char>& vertSPV,
                    const std::vector<char>& fragSPV,
                    const std::vector<VkVertexInputBindingDescription>& bindingDescs,
                    const std::vector<VkVertexInputAttributeDescription>& attrDescs,
                    PipelineConfigInfo& config
                );
                Pipeline(
                    Device& _device,
                    Swapchain& _swapchain,
                    VkPipelineLayout layout,
                    const char* vertFile,
                    const char* fragFile,
                    const std::vector<VkVertexInputBindingDescription>& bindingDescs,
                    const std::vector<VkVertexInputAttributeDescription>& attrDescs,
                    PipelineConfigInfo& config
                );
                Pipeline(
                    Device& _device,
                    Swapchain& _swapchain,
                    VkPipelineLayout layout,
                    ShaderModule& vert,
                    ShaderModule& frag,
                    const std::vector<VkVertexInputBindingDescription>& bindingDescs,
                    const std::vector<VkVertexInputAttributeDescription>& attrDescs,
                    PipelineConfigInfo& config
                );

                Pipeline(Pipeline& p);
                Pipeline(Pipeline&& p);

                ~Pipeline();

                Pipeline& operator = (Pipeline& p){
                    assert(&device == &p.device);
                    device.waitIdle();
                    vkDestroyPipeline(device.device(), _pipeline, nullptr);
                    _pipeline = p._pipeline;
                    p._pipeline = VK_NULL_HANDLE;
                    return *this;
                }
                Pipeline& operator = (Pipeline&& p){
                    assert(&device == &p.device);
                    device.waitIdle();
                    vkDestroyPipeline(device.device(), _pipeline, nullptr);
                    _pipeline = p._pipeline;
                    p._pipeline = VK_NULL_HANDLE;
                    return *this;
                }

                bool valid() { return _pipeline != VK_NULL_HANDLE; }

                void bind(VkCommandBuffer commandBuffer){
                    assert(commandBuffer != VK_NULL_HANDLE);
                    assert(valid());
                    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
                }
            private:
                void init(
                    Swapchain& swapchain,
                    VkPipelineLayout layout,
                    ShaderModule& vert,
                    ShaderModule& frag,
                    const std::vector<VkVertexInputBindingDescription>& bindingDescs,
                    const std::vector<VkVertexInputAttributeDescription>& attrDescs,
                    PipelineConfigInfo& config
                );

                VkPipeline _pipeline;
                Device& device;
        };
    } // namespace gfx
} // namespace shard