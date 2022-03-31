#include <shard/gfx/shaderModule.hpp>

namespace shard{
    namespace gfx{
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
            //assert(sm.valid());
            sm._shaderModule = VK_NULL_HANDLE;
        }
        ShaderModule::ShaderModule(ShaderModule&& sm):
            device{sm.device},
            _shaderModule{sm._shaderModule}
        {
            //assert(sm.valid());
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
    } // namespace gfx
} // namespace shard
