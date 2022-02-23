#include <shard/renderer/textureLoader.hpp>

namespace shard{
    TextureLoader::TextureLoader(
        gfx::Device& _device,
        gfx::DescriptorPool& _descPool
    ):
        device{_device},
        descPool{_descPool}
    {}

    uint32_t TextureLoader::addDescriptorSetLayout(gfx::DescriptorSetLayout&& descLayout){
        descLayouts[currentDescLayoutName] = std::make_unique<gfx::DescriptorSetLayout>(
            descLayout
        );
        return (currentDescLayoutName == UINT32_MAX)
                ? currentDescLayoutName
                : currentDescLayoutName++;
    }
    uint32_t TextureLoader::addSampler(
        VkFilter magFilter, VkFilter minFilter,
        VkSamplerAddressMode U,
        VkSamplerAddressMode V,
        VkSamplerAddressMode W,
        VkBool32 anisotropy,
        VkBorderColor bColor,
        VkSamplerMipmapMode mipMode
    ){
        samplers[currentSamplerName] = std::make_unique<gfx::Sampler>(
            device, 
            magFilter, minFilter,
            U, V, W,
            anisotropy,
            bColor,
            mipMode
        );
        return (currentSamplerName == UINT32_MAX)
            ? currentSamplerName
            : currentSamplerName++;
    }
    uint32_t TextureLoader::addTexture(
        const uint32_t& layout, const uint32_t& sampler,
        const char* filePath, const uint32_t& binding
    ){
        assert(descLayouts.contains(layout));
        assert(samplers.contains(sampler));
        textures[currentTextureName] = std::make_unique<Texture>(
            device, descPool, *descLayouts[layout], *samplers[sampler],
            filePath, binding
        );
        return (currentTextureName == UINT32_MAX)
            ? currentTextureName
            : currentTextureName++;
    }
    uint32_t TextureLoader::addTexture(
        const uint32_t& layout, const uint32_t& sampler,
        uint32_t w, uint32_t h, const void* pixels, const uint32_t& binding
    ){
        assert(pixels != nullptr);
        assert(descLayouts.contains(layout));
        assert(samplers.contains(sampler));
        textures[currentTextureName] = std::make_unique<Texture>(
            device, descPool, *descLayouts[layout], *samplers[sampler],
            w, h, pixels, binding
        );
        return (currentTextureName == UINT32_MAX)
            ? currentTextureName
            : currentTextureName++;
    }

    gfx::DescriptorSetLayout& TextureLoader::getDescriptorSetLayout(const uint32_t& name){
        assert(descLayouts.contains(name));
        return *descLayouts[name];
    }
    gfx::Sampler& TextureLoader::getSampler(const uint32_t& name){
        assert(samplers.contains(name));
        return *samplers[name];
    }
    Texture TextureLoader::getTexture(const uint32_t& name){
        assert(textures.contains(name));
        return *textures[name];
    }
} // namespace shard
