#pragma once

#include "../gfx/gfx.hpp"

namespace shard{
    class Texture{
        public:
            Texture(
                gfx::Device& _device,
                gfx::DescriptorPool& _pool,
                gfx::DescriptorSetLayout& _layout,
                gfx::Sampler& _sampler,
                const char* filePath,
                uint32_t __binding
            );
            Texture(
                gfx::Device& _device,
                gfx::DescriptorPool& _pool,
                gfx::DescriptorSetLayout& _layout,
                gfx::Sampler& _sampler,
                uint32_t w, uint32_t h, const void* pixels,
                uint32_t __binding
            );
            Texture(Texture& t);
            Texture(Texture&& t);
            Texture& operator = (Texture& t);
            Texture& operator = (Texture&& t);

            bool valid() const { return _image.valid() && _descSet != VK_NULL_HANDLE; }
            gfx::Image& image() { return _image; }
            VkDescriptorSet descSet() { return _descSet; }
            uint32_t binding() const { return _binding; }
        private:
            void testCompat(Texture& t);
            void testCompat(Texture&& t);

            gfx::Device& device;
            gfx::DescriptorPool& pool;
            gfx::DescriptorSetLayout& layout;
            gfx::Sampler& sampler;
            gfx::Image _image;
            VkDescriptorSet _descSet;
            uint32_t _binding;
    };
} // namespace shard