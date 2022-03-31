#include <shard/gfx/compute.hpp>

namespace shard{
    namespace gfx
    {
        Compute::Compute(Device& _device):
            device{_device},
            _pipeline{VK_NULL_HANDLE}
        {}
        Compute::Compute(Device& _device, VkPipelineLayout layout, const char* filePath):
            device{_device}
        {
            assert(layout != VK_NULL_HANDLE);
            ShaderModule sm = ShaderModule(device, filePath);
            init(layout, sm);
        }
        Compute::Compute(Device& _device, VkPipelineLayout layout, const std::vector<char>& shaderSPV):
            device{_device}
        {
            assert(layout != VK_NULL_HANDLE);
            ShaderModule sm = ShaderModule(device, shaderSPV);
            init(layout, sm);
        }
        Compute::Compute(Device& _device, VkPipelineLayout layout, ShaderModule& shader):
            device{_device}
        {
            assert(layout != VK_NULL_HANDLE);
            init(layout, shader);
        }
        Compute::Compute(Compute&  c):
            device{c.device},
            _pipeline{c._pipeline}
        {
            c._pipeline = VK_NULL_HANDLE;
        }
        Compute::Compute(Compute&& c):
            device{c.device},
            _pipeline{c._pipeline}
        {
            c._pipeline = VK_NULL_HANDLE;
        }
        Compute::~Compute(){
            vkDestroyPipeline(device.device(), _pipeline, nullptr);
        }

        Compute& Compute::operator = (Compute&  c){
            assert(&device == &c.device);
            vkDestroyPipeline(device.device(), _pipeline, nullptr);
            _pipeline = c._pipeline;
            c._pipeline = VK_NULL_HANDLE;
            return *this;
        }
        Compute& Compute::operator = (Compute&& c){
            assert(&device == &c.device);
            vkDestroyPipeline(device.device(), _pipeline, nullptr);
            _pipeline = c._pipeline;
            c._pipeline = VK_NULL_HANDLE;
            return *this;
        }

        void Compute::init(
            VkPipelineLayout layout,
            ShaderModule& shader
        ){
            VkPipelineShaderStageCreateInfo stageInfo = {};
            stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            stageInfo.module = shader.shaderModule();
            stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            stageInfo.pName = "main";

            VkComputePipelineCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
            createInfo.stage = stageInfo;
            createInfo.layout = layout;

            shard_abort_ifnot(
                vkCreateComputePipelines(
                    device.device(),
                    VK_NULL_HANDLE,
                    1, &createInfo,
                    nullptr,
                    &_pipeline
                ) == VK_SUCCESS
            );
        }
    } // namespace gfx
} // namespace shard
