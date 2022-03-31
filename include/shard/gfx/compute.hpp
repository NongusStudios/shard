#pragma once

#include "shaderModule.hpp"

namespace shard{
    namespace gfx{
        class Compute{
            public:
                Compute(Device& device);
                Compute(Device& device, VkPipelineLayout layout, const char* filePath);
                Compute(Device& device, VkPipelineLayout layout, const std::vector<char>& shaderSPV);
                Compute(Device& device, VkPipelineLayout layout, ShaderModule& shader);
                Compute(Compute&  c);
                Compute(Compute&& c);
                ~Compute();

                Compute& operator = (Compute&  c);
                Compute& operator = (Compute&& c);

                void bind(VkCommandBuffer cmd){
                    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _pipeline);
                }
                void dispatch(VkCommandBuffer cmd, uint32_t x, uint32_t y, uint32_t z=1){
                    vkCmdDispatch(cmd, x, y, z);
                }

                VkPipeline       pipeline()       { return _pipeline; }
                const VkPipeline pipeline() const { return _pipeline; }
            private:
                void init(
                    VkPipelineLayout layout,
                    ShaderModule& shader
                );

                Device& device;
                VkPipeline _pipeline;
        };
    } // namespace gfx
} // namespace shard
