#pragma once

#include <stb_image.h>
#include <vk_mem_alloc.h>

#include "device.hpp"
#include "buffer.hpp"
#include "../def.hpp"
#include "../utils.hpp"

namespace shard{
    namespace gfx{
        class Sampler{
            public:
                Sampler(
                    Device& _device,
                    VkFilter magFilter, VkFilter minFilter,
                    VkSamplerAddressMode U,
                    VkSamplerAddressMode V,
                    VkSamplerAddressMode W,
                    VkBool32 anisotropy,
                    VkBorderColor bColor,
                    VkSamplerMipmapMode mipMode,
                    uint32_t mipLevels
                );
                Sampler(Sampler& s);
                Sampler(Sampler&& s);
                ~Sampler();

                Sampler& operator = (Sampler& s);
                Sampler& operator = (Sampler&& s);

                VkSampler sampler() { return _sampler; }
                bool valid() const { return _sampler != VK_NULL_HANDLE; }
            private:
                Device& device;
                VkSampler _sampler = VK_NULL_HANDLE;
        };
        
        class Image{
            public:
                Image(Device& _device): device{_device} {}
                Image(Device& _device, const char* filePath);
                // Pixel data must be 8bit RGBA
                Image(Device& _device, uint32_t w, uint32_t h, const void* pixels);
                Image(Device& _device,
                    uint32_t w, uint32_t h, uint32_t mipLevels, uint32_t pixelByteSize,
                    VkFormat __format, VkImageTiling tiling,
                    VkSampleCountFlagBits samples,
                    VkImageUsageFlags usage,
                    VkImageCreateFlags flags,
                    VmaMemoryUsage memUsage,
                    VkImageAspectFlags aspectMask,
                    const void* pixels=nullptr
                );
                Image(Image& i);
                Image(Image&& i);
                ~Image();

                shard_delete_copy_constructors(Image);

                Image& operator = (Image& i);
                Image& operator = (Image&& i);

                VkExtent2D extent() const { return _extent; }
                uint32_t pixelSize() const { return _pixelSize; }
                VkFormat format() const { return _format; }
                VkImage image() { return _image; }
                const VkImage image() const { return _image; }
                VkImageView imageView() { return _imageView; }
                const VkImageView imageView() const { return _imageView; }
                VmaAllocation allocation() { return _allocation; }
                const VmaAllocation allocation() const { return _allocation; }
                VkImageAspectFlags imageAspectMask() const { return _aspectMask; }
                uint32_t mipMapLevels() const { return _mipLevels; }
                bool valid() const {
                    return _image      != VK_NULL_HANDLE &&
                           _imageView  != VK_NULL_HANDLE &&
                           _allocation != VK_NULL_HANDLE
                    ;
                }
                VkDescriptorImageInfo descriptorInfo(Sampler& sampler){
                    VkDescriptorImageInfo info = {};
                    info.imageLayout = oldLayout;
                    info.imageView = _imageView;
                    info.sampler = sampler.sampler();
                    return info;
                }

                void transition(VkImageLayout newLayout){
                    device.transitionImageLayout(
                        _image, _format,
                        _mipLevels, _aspectMask,
                        oldLayout, newLayout
                    );
                    oldLayout = newLayout;
                }

                void genMipMaps();
            private:
                void cleanup();

                Device& device;
                VkExtent2D _extent = {};
                VkFormat _format = VK_FORMAT_UNDEFINED;
                uint32_t _pixelSize = 0;
                uint32_t _mipLevels = 0;
                VkImageAspectFlags _aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                VkImage _image = VK_NULL_HANDLE;
                VkImageView _imageView = VK_NULL_HANDLE;
                VmaAllocation _allocation = VK_NULL_HANDLE;
                VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
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