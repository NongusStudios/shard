#include <shard/gfx/image.hpp>
#include <cmath>

namespace shard{
    namespace gfx{
        Image::Image(Device& _device, const char* filePath):
            device{_device}
        {
            int w, h, channels;
            stbi_set_flip_vertically_on_load(true);
            stbi_uc* pixels = stbi_load(filePath, &w, &h, &channels, STBI_rgb_alpha);
            assert(pixels != nullptr && "File does not exist!");

            *this = Image(
                device, 
                uint32_t(w), uint32_t(h), calculateMipmapLevels(uint32_t(w), uint32_t(h)), 4,
                VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                VK_SAMPLE_COUNT_1_BIT,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                VK_IMAGE_USAGE_SAMPLED_BIT,
                0,
                VMA_MEMORY_USAGE_GPU_ONLY,
                VK_IMAGE_ASPECT_COLOR_BIT,
                pixels
            );

            stbi_image_free(pixels);
        }
        Image::Image(Device& _device, uint32_t w, uint32_t h, const void* pixels):
            Image(
                _device, w, h, calculateMipmapLevels(w, h), 4,
                VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                VK_SAMPLE_COUNT_1_BIT,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                VK_IMAGE_USAGE_SAMPLED_BIT,
                0,
                VMA_MEMORY_USAGE_GPU_ONLY,
                VK_IMAGE_ASPECT_COLOR_BIT,
                pixels
            )
        {}
        Image::Image(Device& _device,
            uint32_t w, uint32_t h, uint32_t mipLevels, uint32_t pixelByteSize,
            VkFormat __format, VkImageTiling _tiling,
            VkSampleCountFlagBits samples,
            VkImageUsageFlags usage,
            VkImageCreateFlags flags,
            VmaMemoryUsage memUsage,
            VkImageAspectFlags aspectMask,
            const void* pixels
        ):
            device{_device},
            _extent{w, h},
            _format{__format},
            _pixelSize{pixelByteSize},
            _mipLevels{mipLevels},
            _aspectMask{aspectMask}
        {
            assert(w*h > 0);

            // null buffer
            Buffer stagingBuffer = Buffer(device);
            if(pixels){
                assert(_pixelSize > 0);
                stagingBuffer = Buffer(
                    device, size_t(w*h*pixelByteSize),
                    pixels,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VMA_MEMORY_USAGE_CPU_ONLY,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT  |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
                );
            }

            VkImageCreateInfo imageInfo = {};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = _extent.width;
            imageInfo.extent.height = _extent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = mipLevels;
            imageInfo.arrayLayers = 1;
            imageInfo.format = _format;
            imageInfo.tiling = _tiling;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = usage;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.samples = samples;
            imageInfo.flags = flags;

            VmaAllocationCreateInfo allocInfo = {};
            allocInfo.usage = memUsage;

            shard_abort_ifnot(
                vmaCreateImage(
                    device.allocator(),
                    &imageInfo, &allocInfo,
                    &_image, &_allocation,
                    nullptr
                ) == VK_SUCCESS
            );

            if(pixels){
                transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
                device.copyBufferToImage(
                    stagingBuffer.buffer(), _image,
                    _extent.width, _extent.height 
                );
                if(mipLevels > 1)
                    genMipMaps();
                else
                    transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            }

            VkImageViewCreateInfo imageViewInfo = {};
            imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewInfo.image = _image;
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewInfo.format = _format;
            imageViewInfo.subresourceRange.aspectMask = aspectMask;
            imageViewInfo.subresourceRange.baseMipLevel = 0;
            imageViewInfo.subresourceRange.levelCount = 1;
            imageViewInfo.subresourceRange.baseArrayLayer = 0;
            imageViewInfo.subresourceRange.layerCount = 1;

            shard_abort_ifnot(
                vkCreateImageView(
                    device.device(), &imageViewInfo, nullptr,
                    &_imageView
                ) == VK_SUCCESS
            );            
        }
        Image::Image(Image& i):
            device{i.device},
            _extent{i._extent},
            _format{i._format},
            _pixelSize{i._pixelSize},
            _mipLevels{i._mipLevels},
            _aspectMask{i._aspectMask},
            _image{i._image},
            _imageView{i._imageView},
            _allocation{i._allocation},
            oldLayout{i.oldLayout}
        {
            //assert(i.valid());
            i._extent = {};
            i._format = VK_FORMAT_UNDEFINED;
            i._pixelSize = 0;
            i._mipLevels = 0;
            i._aspectMask = VK_IMAGE_ASPECT_NONE_KHR;
            i._image = VK_NULL_HANDLE;
            i._imageView = VK_NULL_HANDLE;
            i._allocation = VK_NULL_HANDLE;
            i.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        }
        Image::Image(Image&& i):
            device{i.device},
            _extent{i._extent},
            _format{i._format},
            _pixelSize{i._pixelSize},
            _mipLevels{i._mipLevels},
            _aspectMask{i._aspectMask},
            _image{i._image},
            _imageView{i._imageView},
            _allocation{i._allocation},
            oldLayout{i.oldLayout}
        {
            //assert(i.valid());
            i._extent = {};
            i._format = VK_FORMAT_UNDEFINED;
            i._pixelSize = 0;
            i._mipLevels = 0;
            i._aspectMask = VK_IMAGE_ASPECT_NONE_KHR;
            i._image = VK_NULL_HANDLE;
            i._imageView = VK_NULL_HANDLE;
            i._allocation = VK_NULL_HANDLE;
            i.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        }
        void Image::cleanup(){
            vkDestroyImageView(device.device(), _imageView, nullptr);
            vmaDestroyImage(device.allocator(), _image, _allocation);
        }
        Image::~Image(){
            cleanup();
        }
        Image& Image::operator = (Image& i){
            assert(&device == &i.device);
            cleanup();
            _extent     = i._extent;
            _pixelSize  = i._pixelSize;
            _format     = i._format;
            _mipLevels  = i._mipLevels;
            _aspectMask = i._aspectMask;
            _image      = i._image;
            _imageView  = i._imageView;
            _allocation = i._allocation;
            oldLayout   = i.oldLayout;

            i._extent     = {};
            i._format     = VK_FORMAT_UNDEFINED;
            i._mipLevels  = 0;
            i._aspectMask = VK_IMAGE_ASPECT_NONE_KHR;
            i._image      = VK_NULL_HANDLE;
            i._imageView  = VK_NULL_HANDLE;
            i._allocation = VK_NULL_HANDLE;
            i.oldLayout   = VK_IMAGE_LAYOUT_UNDEFINED;

            return *this;
        }
        Image& Image::operator = (Image&& i){
            assert(&device == &i.device);
            cleanup();
            _extent     = i._extent;
            _pixelSize  = i._pixelSize;
            _format     = i._format;
            _mipLevels  = i._mipLevels;
            _aspectMask = i._aspectMask;
            _image      = i._image;
            _imageView  = i._imageView;
            _allocation = i._allocation;
            oldLayout   = i.oldLayout;

            i._extent     = {};
            i._pixelSize  = 0;
            i._format     = VK_FORMAT_UNDEFINED;
            i._mipLevels  = 0;
            i._aspectMask = VK_IMAGE_ASPECT_NONE_KHR;
            i._image      = VK_NULL_HANDLE;
            i._imageView  = VK_NULL_HANDLE;
            i._allocation = VK_NULL_HANDLE;
            i.oldLayout   = VK_IMAGE_LAYOUT_UNDEFINED;

            return *this;
        }

        void Image::genMipMaps(){
            // Check if image format supports linear blitting
            VkFormatProperties formatProperties;
            vkGetPhysicalDeviceFormatProperties(device.pDevice(), _format, &formatProperties);

            shard_abort_ifnot(
                formatProperties.optimalTilingFeatures &
                VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT
            );

            VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();

            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.image = _image;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.subresourceRange.aspectMask = _aspectMask;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.subresourceRange.levelCount = 1;

            int32_t mipWidth = _extent.width;
            int32_t mipHeight = _extent.height;

            for (uint32_t i = 1; i < _mipLevels; i++) {
                barrier.subresourceRange.baseMipLevel = i - 1;
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

                vkCmdPipelineBarrier(commandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier
                );

                VkImageBlit blit{};
                blit.srcOffsets[0] = {0, 0, 0};
                blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
                blit.srcSubresource.aspectMask = _aspectMask;
                blit.srcSubresource.mipLevel = i - 1;
                blit.srcSubresource.baseArrayLayer = 0;
                blit.srcSubresource.layerCount = 1;
                blit.dstOffsets[0] = {0, 0, 0};
                blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
                blit.dstSubresource.aspectMask = _aspectMask;
                blit.dstSubresource.mipLevel = i;
                blit.dstSubresource.baseArrayLayer = 0;
                blit.dstSubresource.layerCount = 1;

                vkCmdBlitImage(commandBuffer,
                    _image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    _image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    1, &blit,
                    VK_FILTER_LINEAR
                );

                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                vkCmdPipelineBarrier(commandBuffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier
                );

                if (mipWidth > 1) mipWidth /= 2;
                if (mipHeight > 1) mipHeight /= 2;
            }

            barrier.subresourceRange.baseMipLevel = _mipLevels - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );

            device.endSingleTimeCommands(commandBuffer);

            oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
         }

        Sampler::Sampler(
            Device& _device,
            VkFilter magFilter, VkFilter minFilter,
            VkSamplerAddressMode U,
            VkSamplerAddressMode V,
            VkSamplerAddressMode W,
            VkBool32 anisotropy,
            VkBorderColor bColor,
            VkSamplerMipmapMode mipMode,
            uint32_t mipLevels
        ):
            device{_device}
        {
            VkSamplerCreateInfo samplerInfo = {};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = magFilter;
            samplerInfo.minFilter = minFilter;
            samplerInfo.addressModeU = U;
            samplerInfo.addressModeV = V;
            samplerInfo.addressModeW = W;
            samplerInfo.anisotropyEnable = anisotropy;
            if(anisotropy)
                samplerInfo.maxAnisotropy = device.properties()
                                            .limits.maxSamplerAnisotropy;
            else samplerInfo.maxAnisotropy = 1.0f;
            samplerInfo.borderColor = bColor;
            samplerInfo.unnormalizedCoordinates = VK_FALSE;
            samplerInfo.compareEnable = VK_FALSE;
            samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
            samplerInfo.mipmapMode = mipMode;
            samplerInfo.minLod = 0.0f;
            samplerInfo.maxLod = static_cast<float>(mipLevels);
            samplerInfo.mipLodBias = 0.0f;
            
            shard_abort_ifnot(
                vkCreateSampler(
                    device.device(), &samplerInfo, nullptr,
                    &_sampler
                ) == VK_SUCCESS
            );
        }
        Sampler::Sampler(Sampler& s):
            device{s.device},
            _sampler{s._sampler}
        {
            assert(s.valid());
            s._sampler = VK_NULL_HANDLE;
        }
        Sampler::Sampler(Sampler&& s):
            device{s.device},
            _sampler{s._sampler}
        {
            assert(s.valid());
            s._sampler = VK_NULL_HANDLE;
        }
        Sampler::~Sampler(){
            vkDestroySampler(device.device(), _sampler, nullptr);
        }

        Sampler& Sampler::operator = (Sampler& s){
            assert(&device == &s.device);
            _sampler = s._sampler;
            s._sampler = VK_NULL_HANDLE;
            return *this;
        }
        Sampler& Sampler::operator = (Sampler&& s){
            assert(&device == &s.device);
            _sampler = s._sampler;
            s._sampler = VK_NULL_HANDLE;
            return *this;
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