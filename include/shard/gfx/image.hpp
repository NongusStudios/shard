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
                    VkSamplerMipmapMode mipMode
                );
                Sampler(Sampler& s);
                Sampler(Sampler&& s);
                ~Sampler();

                Sampler& operator = (Sampler& s);
                Sampler& operator = (Sampler&& s);

                VkSampler sampler() { return _sampler; }
                bool valid() { return _sampler != VK_NULL_HANDLE; }
            private:
                Device& device;
                VkSampler _sampler = VK_NULL_HANDLE;
        };
        
        class Image{
            public:
                Image(Device& _device): device{_device} {}
                Image(Device& _device, const char* filePath);
                Image(Device& _device,
                    uint32_t w, uint32_t h, uint32_t mipLevels,
                    VkFormat __format, VkImageTiling tiling,
                    VkSampleCountFlagBits samples,
                    VkImageUsageFlags usage,
                    VkImageCreateFlags flags,
                    VmaMemoryUsage memUsage,
                    const void* pixels=nullptr
                );
                Image(Image& i);
                Image(Image&& i);
                ~Image();
                Image& operator = (Image& i);
                Image& operator = (Image&& i);

                VkExtent2D extent() { return _extent; }
                VkFormat format() { return _format; }
                VkImage image() { return _image; }
                VkImageView imageView() { return _imageView; }
                VmaAllocation allocation() { return _allocation; }
                bool valid(){
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
                        oldLayout, newLayout
                    );
                    oldLayout = newLayout;
                }
            private:
                void cleanup();

                Device& device;
                VkExtent2D _extent = {};
                VkFormat _format = VK_FORMAT_UNDEFINED;
                VkImage _image = VK_NULL_HANDLE;
                VkImageView _imageView = VK_NULL_HANDLE;
                VmaAllocation _allocation = VK_NULL_HANDLE;

                VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        };
    } // namespace gfx
} // namespace shard
