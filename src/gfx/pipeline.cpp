#include <shard/gfx/pipeline.hpp>

namespace shard{
    namespace gfx{
        void PipelineConfigInfo::makeDefault(){
            inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

            viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportInfo.viewportCount = 1;
            viewportInfo.pViewports = nullptr;
            viewportInfo.scissorCount = 1;
            viewportInfo.pScissors = nullptr;

            rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizationInfo.depthClampEnable = VK_FALSE;
            rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
            rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizationInfo.lineWidth = 1.0f;
            rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
            rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterizationInfo.depthBiasEnable = VK_FALSE;
            rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
            rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
            rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

            multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampleInfo.sampleShadingEnable = VK_FALSE;
            multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            multisampleInfo.minSampleShading = 1.0f;           // Optional
            multisampleInfo.pSampleMask = nullptr;             // Optional
            multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
            multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

            colorBlendAttachment.colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable = VK_FALSE;
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
            colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
            colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

            colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlendInfo.logicOpEnable = VK_FALSE;
            colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
            colorBlendInfo.attachmentCount = 1;
            colorBlendInfo.pAttachments = &colorBlendAttachment;
            colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
            colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
            colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
            colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

            depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencilInfo.depthTestEnable = VK_TRUE;
            depthStencilInfo.depthWriteEnable = VK_TRUE;
            depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
            depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
            depthStencilInfo.minDepthBounds = 0.0f;  // Optional
            depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
            depthStencilInfo.stencilTestEnable = VK_FALSE;
            depthStencilInfo.front = {};  // Optional
            depthStencilInfo.back = {};   // Optional

            dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
            dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicStateInfo.pDynamicStates = dynamicStateEnables.data();
            dynamicStateInfo.dynamicStateCount =
                static_cast<uint32_t>(dynamicStateEnables.size());
            dynamicStateInfo.flags = 0;
        }
        void PipelineConfigInfo::makeWireframe(){
            makeDefault();
            rasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE;
        }

        ShaderModule::ShaderModule(Device& _device, const char* filePath):
            device{_device}
        {
            auto src = shard::readSPVfile(filePath);
            init(src);
        }
        ShaderModule::ShaderModule(Device& _device, const std::vector<char>& spv):
            device{_device}
        {
            init(spv);
        }
        ShaderModule::ShaderModule(ShaderModule& sm):
            device{sm.device},
            _shaderModule{sm._shaderModule}
        {
            assert(sm.valid());
            sm._shaderModule = VK_NULL_HANDLE;
        }
        ShaderModule::ShaderModule(ShaderModule&& sm):
            device{sm.device},
            _shaderModule{sm._shaderModule}
        {
            assert(sm.valid());
            sm._shaderModule = VK_NULL_HANDLE;
        }
        ShaderModule::~ShaderModule(){
            vkDestroyShaderModule(device.device(), _shaderModule, nullptr);
        }

        void ShaderModule::init(const std::vector<char>& spv){
            VkShaderModuleCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = spv.size();
            createInfo.pCode = reinterpret_cast<const uint32_t*>(spv.data());

            shard_abort_ifnot(
                vkCreateShaderModule(
                    device.device(), &createInfo, nullptr,
                    &_shaderModule
                ) == VK_SUCCESS
            );
        }
        
        Pipeline::Pipeline(
            Device& _device,
            VkRenderPass _renderPass,
            VkPipelineLayout layout,
            const std::vector<char>& vertSPV,
            const std::vector<char>& fragSPV,
            const std::vector<VkVertexInputBindingDescription>& bindingDescs,
            const std::vector<VkVertexInputAttributeDescription>& attrDescs,
            PipelineConfigInfo& config
        ):
            device{_device}
        {
            assert(layout != VK_NULL_HANDLE);
            ShaderModule vert(device, vertSPV);
            ShaderModule frag(device, fragSPV);
            init(_renderPass,
                layout, vert, frag,
                bindingDescs,
                attrDescs,
                config
            );
        }
        Pipeline::Pipeline(
            Device& _device,
            VkRenderPass _renderPass,
            VkPipelineLayout layout,
            const char* vertFile,
            const char* fragFile,
            const std::vector<VkVertexInputBindingDescription>& bindingDescs,
            const std::vector<VkVertexInputAttributeDescription>& attrDescs,
            PipelineConfigInfo& config
        ):
            device{_device}
        {
            assert(layout != VK_NULL_HANDLE);
            ShaderModule vert(device, vertFile);
            ShaderModule frag(device, fragFile);
            init(_renderPass,
                layout, vert, frag,
                bindingDescs,
                attrDescs,
                config
            );
        }
        Pipeline::Pipeline(
            Device& _device,
            VkRenderPass _renderPass,
            VkPipelineLayout layout,
            ShaderModule& vert,
            ShaderModule& frag,
            const std::vector<VkVertexInputBindingDescription>& bindingDescs,
            const std::vector<VkVertexInputAttributeDescription>& attrDescs,
            PipelineConfigInfo& config
        ):
            device{_device}
        {
            assert(layout != VK_NULL_HANDLE);
            assert(vert.valid());
            assert(frag.valid());
            init(_renderPass,
                layout, vert, frag,
                bindingDescs,
                attrDescs,
                config
            );
        }
        Pipeline::Pipeline(Pipeline& p):
            device{p.device},
            _pipeline{p._pipeline}
        {
            assert(p.valid());
            p._pipeline = VK_NULL_HANDLE;
        }
        Pipeline::Pipeline(Pipeline&& p):
            device{p.device},
            _pipeline{p._pipeline}
        {
            assert(p.valid());
            p._pipeline = VK_NULL_HANDLE;
        }
        Pipeline::~Pipeline(){
            vkDestroyPipeline(device.device(), _pipeline, nullptr);
        }
        void Pipeline::init(
            VkRenderPass renderPass,
            VkPipelineLayout layout,
            ShaderModule& vert,
            ShaderModule& frag,
            const std::vector<VkVertexInputBindingDescription>& bindingDescs,
            const std::vector<VkVertexInputAttributeDescription>& attrDescs,
            PipelineConfigInfo& config
        ){
            VkPipelineShaderStageCreateInfo shaderStages[2] = {{}};
            shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStages[0].module = vert.shaderModule();
            shaderStages[0].pName = "main";
            shaderStages[0].flags = 0;
            shaderStages[0].pNext = nullptr;
            shaderStages[0].pSpecializationInfo = nullptr;

            shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            shaderStages[1].module = frag.shaderModule();
            shaderStages[1].pName = "main";
            shaderStages[1].flags = 0;
            shaderStages[1].pNext = nullptr;
            shaderStages[1].pSpecializationInfo = nullptr;

            VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(
                attrDescs.size()
            );
            vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(
                bindingDescs.size()
            );
            vertexInputInfo.pVertexAttributeDescriptions = attrDescs.data();
            vertexInputInfo.pVertexBindingDescriptions = bindingDescs.data();

            VkGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = 2;
            pipelineInfo.pStages = shaderStages;
            pipelineInfo.pVertexInputState = &vertexInputInfo;
            pipelineInfo.pInputAssemblyState = &config.inputAssemblyInfo;
            pipelineInfo.pViewportState = &config.viewportInfo;
            pipelineInfo.pRasterizationState = &config.rasterizationInfo;
            pipelineInfo.pMultisampleState = &config.multisampleInfo;
            pipelineInfo.pColorBlendState = &config.colorBlendInfo;
            pipelineInfo.pDepthStencilState = &config.depthStencilInfo;
            pipelineInfo.pDynamicState = &config.dynamicStateInfo;

            pipelineInfo.layout = layout;
            pipelineInfo.renderPass = renderPass;
            pipelineInfo.subpass = config.subpass;

            pipelineInfo.basePipelineIndex = -1;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

            shard_abort_ifnot(
                vkCreateGraphicsPipelines(
                    device.device(),
                    VK_NULL_HANDLE,
                    1, &pipelineInfo,
                    nullptr,
                    &_pipeline
                ) == VK_SUCCESS
            );
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