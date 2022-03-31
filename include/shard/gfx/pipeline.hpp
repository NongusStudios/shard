#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <memory>

#include "../def.hpp"
#include "../utils.hpp"

#include "device.hpp"
#include "swapchain.hpp"
#include "shaderModule.hpp"

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

        class Pipeline{
            public:
                Pipeline(Device& _device):
                    device{_device}
                {}
                Pipeline(
                    Device& _device,
                    VkRenderPass _renderPass,
                    VkPipelineLayout layout,
                    const std::vector<char>& vertSPV,
                    const std::vector<char>& fragSPV,
                    const std::vector<VkVertexInputBindingDescription>& bindingDescs,
                    const std::vector<VkVertexInputAttributeDescription>& attrDescs,
                    PipelineConfigInfo& config
                );
                Pipeline(
                    Device& _device,
                    VkRenderPass _renderPass,
                    VkPipelineLayout layout,
                    const char* vertFile,
                    const char* fragFile,
                    const std::vector<VkVertexInputBindingDescription>& bindingDescs,
                    const std::vector<VkVertexInputAttributeDescription>& attrDescs,
                    PipelineConfigInfo& config
                );
                Pipeline(
                    Device& _device,
                    VkRenderPass _renderPass,
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

                shard_delete_copy_constructors(Pipeline);

                Pipeline& operator = (Pipeline& p){
                    assert(&device == &p.device);
                    vkDestroyPipeline(device.device(), _pipeline, nullptr);
                    _pipeline = p._pipeline;
                    p._pipeline = VK_NULL_HANDLE;
                    return *this;
                }
                Pipeline& operator = (Pipeline&& p){
                    assert(&device == &p.device);
                    vkDestroyPipeline(device.device(), _pipeline, nullptr);
                    _pipeline = p._pipeline;
                    p._pipeline = VK_NULL_HANDLE;
                    return *this;
                }

                VkPipeline pipeline(){ return _pipeline; }
                const VkPipeline pipeline() const { return _pipeline; }

                bool valid() const { return _pipeline != VK_NULL_HANDLE; }

                void bind(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint){
                    assert(commandBuffer != VK_NULL_HANDLE);
                    assert(valid());
                    vkCmdBindPipeline(commandBuffer, bindPoint, _pipeline);
                }
            private:
                void init(
                    VkRenderPass renderPass,
                    VkPipelineLayout layout,
                    ShaderModule& vert,
                    ShaderModule& frag,
                    const std::vector<VkVertexInputBindingDescription>& bindingDescs,
                    const std::vector<VkVertexInputAttributeDescription>& attrDescs,
                    PipelineConfigInfo& config
                );
                Device& device;
                VkPipeline _pipeline=VK_NULL_HANDLE;
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