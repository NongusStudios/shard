#pragma once

#include "texture.hpp"

namespace shard{
    class TextureLoader{
        public:
            TextureLoader(
                gfx::Device& _device,
                gfx::DescriptorPool& _descPool
            );

            uint32_t addDescriptorSetLayout(gfx::DescriptorSetLayout&& descLayout);
            uint32_t addSampler(
                VkFilter magFilter, VkFilter minFilter,
                VkSamplerAddressMode U,
                VkSamplerAddressMode V,
                VkSamplerAddressMode W,
                VkBool32 anisotropy,
                VkBorderColor bColor,
                VkSamplerMipmapMode mipMode
            );
            uint32_t addTexture(
                const uint32_t& layout, const uint32_t& sampler,
                const char* filePath, const uint32_t& binding
            );
            uint32_t addTexture(
                const uint32_t& layout, const uint32_t& sampler,
                uint32_t w, uint32_t h, const void* pixels, const uint32_t& binding
            );

            gfx::DescriptorSetLayout& getDescriptorSetLayout(const uint32_t& name);
            gfx::Sampler& getSampler(const uint32_t& name);
            Texture getTexture(const uint32_t& name);
        private:
            gfx::Device& device;
            gfx::DescriptorPool& descPool;
            std::map<uint32_t, std::unique_ptr<gfx::DescriptorSetLayout>> descLayouts;
            std::map<uint32_t, std::unique_ptr<gfx::Sampler>> samplers;
            std::map<uint32_t, std::unique_ptr<Texture>> textures;

            uint32_t currentDescLayoutName = 1;
            uint32_t currentSamplerName = 1;
            uint32_t currentTextureName = 1;
    };
} // namespace shard
