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