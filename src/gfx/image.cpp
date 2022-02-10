#include <shard/gfx/image.hpp>

namespace shard{
    namespace gfx{
        Image::Image(Device& _device, const char* filePath):
            device{_device}
        {
            int w, h, channels;
            stbi_uc* pixels = stbi_load(filePath, &w, &h, &channels, STBI_rgb_alpha);
            
            *this = Image(
                device, 
                uint32_t(w), uint32_t(h), 1,
                VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                VK_SAMPLE_COUNT_1_BIT,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                0,
                VMA_MEMORY_USAGE_GPU_ONLY,
                pixels
            );

            stbi_image_free(pixels);
        }
        Image::Image(Device& _device,
            uint32_t w, uint32_t h, uint32_t mipLevels,
            VkFormat __format, VkImageTiling _tiling,
            VkSampleCountFlagBits samples,
            VkImageUsageFlags usage,
            VkImageCreateFlags flags,
            VmaMemoryUsage memUsage,
            const void* pixels
        ):
            device{_device},
            _extent{w, h},
            _format{__format}
        {
            assert(w*h > 0);

            // null buffer
            Buffer stagingBuffer = Buffer(device);
            if(pixels){
                stagingBuffer = Buffer(
                    device, size_t(w*h*4),
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
                transition(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            }

            VkImageViewCreateInfo imageViewInfo = {};
            imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewInfo.image = _image;
            imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewInfo.format = _format;
            imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
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
            _image{i._image},
            _imageView{i._imageView},
            _allocation{i._allocation},
            oldLayout{i.oldLayout}
        {
            assert(i.valid());
            i._extent = {};
            i._format = VK_FORMAT_UNDEFINED;
            i._image = VK_NULL_HANDLE;
            i._imageView = VK_NULL_HANDLE;
            i._allocation = VK_NULL_HANDLE;
            i.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        }
        Image::Image(Image&& i):
            device{i.device},
            _extent{i._extent},
            _format{i._format},
            _image{i._image},
            _imageView{i._imageView},
            _allocation{i._allocation},
            oldLayout{i.oldLayout}
        {
            assert(i.valid());
            i._extent = {};
            i._format = VK_FORMAT_UNDEFINED;
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
            _format     = i._format;
            _image      = i._image;
            _imageView  = i._imageView;
            _allocation = i._allocation;
            oldLayout   = i.oldLayout;

            i._extent     = {};
            i._format     = VK_FORMAT_UNDEFINED;
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
            _format     = i._format;
            _image      = i._image;
            _imageView  = i._imageView;
            _allocation = i._allocation;
            oldLayout   = i.oldLayout;

            i._extent     = {};
            i._format     = VK_FORMAT_UNDEFINED;
            i._image      = VK_NULL_HANDLE;
            i._imageView  = VK_NULL_HANDLE;
            i._allocation = VK_NULL_HANDLE;
            i.oldLayout   = VK_IMAGE_LAYOUT_UNDEFINED;

            return *this;
        }

        Sampler::Sampler(
            Device& _device,
            VkFilter magFilter, VkFilter minFilter,
            VkSamplerAddressMode U,
            VkSamplerAddressMode V,
            VkSamplerAddressMode W,
            VkBool32 anisotropy,
            VkBorderColor bColor,
            VkSamplerMipmapMode mipMode
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
            samplerInfo.mipLodBias = 0.0f;
            samplerInfo.minLod = 0.0f;
            samplerInfo.maxLod = 0.0f;
            
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
