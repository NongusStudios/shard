#include <shard/renderer/textureLoader.hpp>

namespace shard{
    TextureLoader::TextureLoader(
        gfx::Device& _device,
        gfx::DescriptorPool& _descPool
    ):
        device{_device},
        descPool{_descPool}
    {}
    uint32_t TextureLoader::addTexture(
        gfx::DescriptorSetLayout& layout, gfx::Sampler& sampler,
        const char* filePath, const uint32_t& binding
    ){
        textures[currentTextureName] = std::make_unique<Texture>(
            device, descPool, layout, sampler,
            filePath, binding
        );
        return currentTextureName++;
    }
    uint32_t TextureLoader::addTexture(
        gfx::DescriptorSetLayout& layout, gfx::Sampler& sampler,
        uint32_t w, uint32_t h, const void* pixels, const uint32_t& binding
    ){
        assert(pixels != nullptr);
        textures[currentTextureName] = std::make_unique<Texture>(
            device, descPool, layout, sampler,
            w, h, pixels, binding
        );
        return currentTextureName++;
    }

    Texture& TextureLoader::getTexture(const uint32_t& name){
        assert(textures.contains(name));
        return *textures[name];
    }
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