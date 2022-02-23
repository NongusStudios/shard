#include <shard/renderer/texture.hpp>

namespace shard{
    Texture::Texture(
        gfx::Device& _device,
        gfx::DescriptorPool& _pool,
        gfx::DescriptorSetLayout& _layout,
        gfx::Sampler& _sampler,
        const char* filePath,
        uint32_t __binding
    ):
        device{_device},
        pool{_pool},
        layout{_layout},
        sampler{_sampler},
        _image{device, filePath},
        _descSet{VK_NULL_HANDLE},
        _binding{__binding}
    {
        auto descInfo = _image.descriptorInfo(sampler);
        gfx::DescriptorWriter(layout, pool)
            .writeImage(_binding, &descInfo)
            .build(_descSet);
    }
    Texture::Texture(
        gfx::Device& _device,
        gfx::DescriptorPool& _pool,
        gfx::DescriptorSetLayout& _layout,
        gfx::Sampler& _sampler,
        uint32_t w, uint32_t h, const void* pixels,
        uint32_t __binding
    ):
        device{_device},
        pool{_pool},
        layout{_layout},
        sampler{_sampler},
        _image{device, w, h, pixels},
        _descSet{VK_NULL_HANDLE},
        _binding{__binding}
    {
        auto descInfo = _image.descriptorInfo(sampler);
        gfx::DescriptorWriter(layout, pool)
            .writeImage(_binding, &descInfo)
            .build(_descSet);
    }
    Texture::Texture(Texture& t):
        device{t.device},
        pool{t.pool},
        layout{t.layout},
        sampler{t.sampler},
        _image{t._image},
        _descSet{t._descSet},
        _binding{t._binding}
    {
        t._descSet = VK_NULL_HANDLE;
        t._binding = 0;
    }
    Texture::Texture(Texture&& t):
        device{t.device},
        pool{t.pool},
        layout{t.layout},
        sampler{t.sampler},
        _image{t._image},
        _descSet{t._descSet},
        _binding{t._binding}
    {
        t._descSet = VK_NULL_HANDLE;
        t._binding = 0;
    }
    Texture& Texture::operator = (Texture& t){
        testCompat(t);
        _image = t._image;
        _descSet = t._descSet;
        _binding = t._binding;

        t._descSet = VK_NULL_HANDLE;
        t._binding = 0;
        return *this;
    }
    Texture& Texture::operator = (Texture&& t){
        testCompat(t);
        _image = t._image;
        _descSet = t._descSet;
        _binding = t._binding;

        t._descSet = VK_NULL_HANDLE;
        t._binding = 0;
        return *this;
    }

    void Texture::testCompat(Texture& t){
        assert(&device == &t.device);
        assert(&pool == &t.pool);
        assert(&layout == &t.layout);
        assert(&sampler == &t.sampler);
    }
    void Texture::testCompat(Texture&& t){
        assert(&device == &t.device);
        assert(&pool == &t.pool);
        assert(&layout == &t.layout);
        assert(&sampler == &t.sampler);
    }
} // namespace shard