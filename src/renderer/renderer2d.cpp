#include <shard/renderer/renderer2d.hpp>

#include <memory.h>

namespace shard{
    namespace r2d{
        const gfx::Vertex2D Rect::VERTICES[4] = {
            //            pos            
            gfx::Vertex2D({-1.0f, -1.0f}, {0.0f, 0.0f}, {1.0f}),
            gfx::Vertex2D({ 1.0f,  1.0f}, {1.0f, 1.0f}, {1.0f}),
            gfx::Vertex2D({-1.0f,  1.0f}, {0.0f, 1.0f}, {1.0f}),
            gfx::Vertex2D({ 1.0f, -1.0f}, {1.0f, 0.0f}, {1.0f})
        };
        const uint32_t      Rect::INDICES[6] = {
            0, 1, 2,
            0, 3, 1
        };

        gfx::Model Rect::createRectModel(gfx::Graphics& gfx){
            return gfx::Model(
                gfx,
                VERTICES, 4, INDICES, 6
            );
        }

        Rect::Rect(
            gfx::Graphics& gfx,
            gfx::DescriptorPool& pool,
            gfx::DescriptorSetLayout& layout,
            const uint32_t binding,
            const glm::vec2& pos,
            const float rot,
            const glm::vec2& _scale,
            const gfx::Color& _color,
            const float _zindex,
            const bool _hasBorder,
            const float _borderSize,
            const gfx::Color& _borderColor
        ):
            position{pos},
            rotation{rot},
            scale{_scale},
            color{_color},
            zindex{_zindex},
            hasBorder{_hasBorder},
            borderSize{_borderSize},
            borderColor{_borderColor}
        {
            _descSets.resize(gfx::Swapchain::MAX_FRAMES_IN_FLIGHT);
            for(uint32_t i = 0; i < gfx::Swapchain::MAX_FRAMES_IN_FLIGHT; i++){
                uBuffers.push_back(gfx::Buffer(
                    gfx.createUniformBuffer(
                        sizeof(UBO), nullptr
                    )
                ));
                uBuffers[i].map();
                auto bufferInfo = uBuffers[i].descriptorInfo();
                gfx::DescriptorWriter(layout, pool)
                    .writeBuffer(binding, &bufferInfo)
                    .build(_descSets[i]);
            }
        }

        void Rect::bind(
            VkCommandBuffer commandBuffer,
            VkDescriptorSet constDescSet,
            VkPipelineLayout pLayout,
            uint32_t frameIndex
        ){
            UBO ubo = {};
            ubo.model = this->modelMatrix();
            ubo.color = color;
            ubo.hasBorder = hasBorder;
            ubo.borderSize = borderSize;
            ubo.borderColor = borderColor;
            ubo.rectSize = scale;
            
            memcpy(
                uBuffers[frameIndex].mappedMemory(),
                &ubo,
                sizeof(UBO)
            );                       // constant     // variable
            VkDescriptorSet descSets[] = {constDescSet, _descSets[frameIndex]};
            vkCmdBindDescriptorSets(commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pLayout,
                0, 2,
                descSets,
                0, nullptr
            );
        }

        void Rect::cleanup(gfx::DescriptorPool& descPool){
            descPool.freeDescriptors(
                _descSets
            );
        }
        glm::mat4 Rect::modelMatrix() const {
            glm::mat4 m = glm::mat4(1.0f);
            m = glm::translate(m, {position, zindex});
            m = glm::rotate(m, rotation, {0.0f, 0.0f, -1.0f});
            m = glm::scale(m, {scale, 0.0f});
            return m;
        }

        Sprite::Sprite(
            gfx::Graphics& gfx,
            gfx::DescriptorPool& pool,
            gfx::DescriptorSetLayout& layout,
            gfx::Image& image,
            gfx::Sampler& sampler,
            uint32_t binding,
            uint32_t texBinding,
            const glm::vec2& pos,
            float rot,
            const glm::vec2& _scale,
            const gfx::Color& _color,
            float _zindex,
            const glm::vec2& srcPos,
            const glm::vec2& srcSize
        ):
            position{pos},
            rotation{rot},
            scale{_scale},
            color{_color},
            zindex{_zindex},
            srcRect{
                srcPos,
                srcSize
            }
        {
            _descSets.resize(gfx::Swapchain::MAX_FRAMES_IN_FLIGHT);
            for(uint32_t i = 0; i < gfx::Swapchain::MAX_FRAMES_IN_FLIGHT; i++){
                uBuffers.push_back(gfx::Buffer(
                    gfx.createUniformBuffer(
                        sizeof(UBO), nullptr
                    )
                ));
                uBuffers[i].map();

                auto bufferInfo = uBuffers[i].descriptorInfo();
                auto imageInfo = image.descriptorInfo(sampler);
                gfx::DescriptorWriter(layout, pool)
                    .writeBuffer(binding, &bufferInfo)
                    .writeImage(texBinding, &imageInfo)
                    .build(_descSets[i]);
            }
        }

        void Sprite::bind(
            VkCommandBuffer commandBuffer,
            VkDescriptorSet constDescSet,
            VkPipelineLayout pLayout,
            uint32_t frameIndex
        ){
            UBO ubo = {};
            ubo.model       = this->modelMatrix();
            ubo.color       = color;
            ubo.srcRectPos  = srcRect.position;
            ubo.srcRectSize = srcRect.size;

            memcpy(
                uBuffers[frameIndex].mappedMemory(),
                &ubo,
                sizeof(UBO)
            );                       // constant     // variable
            VkDescriptorSet descSets[] = {constDescSet, _descSets[frameIndex]};
            vkCmdBindDescriptorSets(commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pLayout,
                0, 2,
                descSets,
                0, nullptr
            );
        }
        glm::mat4 Sprite::modelMatrix() const {
            glm::mat4 m = glm::mat4(1.0f);
            m = glm::translate(m, {position, zindex});
            m = glm::rotate(m, rotation, {0.0f, 0.0f, -1.0f});
            m = glm::scale(m, {scale, 0.0f});
            return m;
        }

        void Sprite::cleanup(gfx::DescriptorPool& descPool){
            descPool.freeDescriptors(
                _descSets
            );
        }

        // Renderer begin
        Renderer::Renderer(
            GLFWwindow* win,
            VkExtent2D renderExtent,
            uint32_t texturePoolSize,
            bool _vsync
        ):
            window{win},
            gfx{window, _vsync},
            descPool{gfx::DescriptorPool::Builder(gfx.device())
                .addPoolSize(
                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                    gfx::Swapchain::MAX_FRAMES_IN_FLIGHT+UBUFFER_POOL_SIZE
                )
                .addPoolSize(
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    texturePoolSize
                )
                .setMaxSets(gfx::Swapchain::MAX_FRAMES_IN_FLIGHT+UBUFFER_POOL_SIZE)
                .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
                .build()
            },
            layouts{
                gfx::DescriptorSetLayout::Builder(gfx.device())
                    .addBinding(
                        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT
                    )
                    .build(),
                gfx::DescriptorSetLayout::Builder(gfx.device())
                    .addBinding(
                        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        VK_SHADER_STAGE_VERTEX_BIT |
                        VK_SHADER_STAGE_FRAGMENT_BIT
                    )
                    .build(),
                gfx::DescriptorSetLayout::Builder(gfx.device())
                    .addBinding(
                        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        VK_SHADER_STAGE_VERTEX_BIT |
                        VK_SHADER_STAGE_FRAGMENT_BIT
                    )
                    .addBinding(
                        1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        VK_SHADER_STAGE_FRAGMENT_BIT
                    )
                    .build(),
                
                createPipelineLayout(
                    {
                        layouts.constant.layout(),
                        layouts.rect.layout()
                    }
                ),
                createPipelineLayout(
                    {
                        layouts.constant.layout(),
                        layouts.sprite.layout()
                    }
                )
            },
            pipelines{
                gfx.createPipeline(
                    layouts.plRect,
                    "shaders/r2d/rect.vert.spv",
                    "shaders/r2d/rect.frag.spv",
                    {gfx::Vertex2D::bindingDesc(
                        VK_VERTEX_INPUT_RATE_VERTEX
                    )},
                    gfx::Vertex2D::attributeDescs(),
                    gfx.deafultPipelineConfig()
                ),
                gfx.createPipeline(
                    layouts.plSprite,
                    "shaders/r2d/sprite.vert.spv",
                    "shaders/r2d/sprite.frag.spv",
                    {gfx::Vertex2D::bindingDesc(
                        VK_VERTEX_INPUT_RATE_VERTEX
                    )},
                    gfx::Vertex2D::attributeDescs(),
                    gfx.deafultPipelineConfig()
                )
            },
            models{
                Rect::createRectModel(gfx)
            },
            extent{renderExtent},
            vsync{_vsync}
        {
            constantDescSets.resize(gfx::Swapchain::MAX_FRAMES_IN_FLIGHT);
            for(uint32_t i = 0; i < gfx::Swapchain::MAX_FRAMES_IN_FLIGHT; i++){
                constantUniformBuffers.push_back(
                    gfx.createUniformBuffer(
                        sizeof(UBO), nullptr
                    )
                );
                constantUniformBuffers[i].map();

                auto bufferInfo
                     = constantUniformBuffers[i].descriptorInfo();
                gfx::DescriptorWriter(layouts.constant, descPool)
                    .writeBuffer(0, &bufferInfo)
                    .build(constantDescSets[i]);
            }
        }

        bool Renderer::startFrame(const gfx::Color& color){
            assert(currentCommandBuffer == VK_NULL_HANDLE);

            if((currentCommandBuffer = gfx.beginRenderPass(color))){
                UBO ubo;
                ubo.projection = getProjectionMatrix();
                ubo.view = glm::mat4(1.0f);
                memcpy(
                    constantUniformBuffers[gfx.frameIndex()].mappedMemory(),
                    &ubo,
                    sizeof(UBO)
                );
                return true;
            }
            return false;
        }
        void Renderer::endFrame(){
            assert(currentCommandBuffer != VK_NULL_HANDLE);
            gfx.endRenderPass();
            currentCommandBuffer = VK_NULL_HANDLE;
        }

        uint32_t Renderer::addRect(
            const glm::vec2& position,
            float rotation,
            const glm::vec2& scale,
            const gfx::Color& color,
            float zindex
        ){
            rects[names.rect] = Rect(
                gfx, descPool,
                layouts.rect, 0,
                position, rotation, scale,
                color, zindex
            );

            return names.rect++;
        }
        uint32_t Renderer::addRect(
            const glm::vec2& position,
            float rotation,
            const glm::vec2& scale,
            const gfx::Color& color,
            float zindex,
            float borderSize,
            const gfx::Color& borderColor
        ){
            rects[names.rect] = Rect(
                gfx, descPool,
                layouts.rect, 0,
                position, rotation, scale,
                color, zindex,
                true,
                borderSize, borderColor
            );

            return names.rect++;
        }
        Rect& Renderer::getRect(uint32_t name){
            assert(rects.contains(name));
            return rects[name];
        }
        void Renderer::removeRect(uint32_t name){
            assert(rects.contains(name));
            rects[name].cleanup(descPool);
            rects.erase(name);
        }

        uint32_t Renderer::addTexture(const char* filePath, VkFilter filter){
            gfx::Image tex = gfx.createTexture(filePath);
            uint32_t texMipMap = tex.mipMapLevels();
            textures[names.texture] = std::make_unique<Texture>(
                tex,
                gfx.createSampler(
                    filter,
                    filter,
                    VK_SAMPLER_ADDRESS_MODE_REPEAT,
                    VK_SAMPLER_ADDRESS_MODE_REPEAT,
                    VK_SAMPLER_ADDRESS_MODE_REPEAT,
                    VK_TRUE,
                    VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                    VK_SAMPLER_MIPMAP_MODE_LINEAR, texMipMap
                )
            );
            return names.texture++;
        }
        Renderer::Texture& Renderer::getTexture(uint32_t name){
            assert(textures.contains(name));
            return *textures[name];
        }
        void Renderer::removeTexture(uint32_t name){
            assert(textures.contains(name));
            textures.erase(name);
        }

        uint32_t Renderer::addSprite(
            uint32_t texture,
            const glm::vec2& position,
            float rotation,
            const glm::vec2& scale,
            const gfx::Color& color,
            float zindex
        ){
            Texture& tex = getTexture(texture);
            sprites[names.sprite] = Sprite(
                gfx,
                descPool,
                layouts.sprite,
                tex.image, tex.sampler,
                0, 1,
                position, rotation, scale,
                color, zindex,
                {0.0f, 0.0f},
                {1.0f, 1.0f}
            );
            return names.sprite++;
        }
        uint32_t Renderer::addSprite(
            uint32_t texture,
            const glm::vec2& position,
            float rotation,
            const glm::vec2& scale,
            const gfx::Color& color,
            float zindex,
            const glm::vec2& srcPos,
            const glm::vec2& srcSize
        ){
            Texture& tex = getTexture(texture);
            sprites[names.sprite] = Sprite(
                gfx,
                descPool,
                layouts.sprite,
                tex.image, tex.sampler,
                0, 1,
                position, rotation, scale,
                color, zindex,
                srcPos,
                srcSize
            );
            return names.sprite++;
        }
        Sprite& Renderer::getSprite(uint32_t sprite){
            assert(sprites.contains(sprite));
            return sprites[sprite];
        }
        void Renderer::removeSprite(uint32_t sprite){
            assert(sprites.contains(sprite));
            Sprite& s = getSprite(sprite);
            s.cleanup(descPool);
            sprites.erase(sprite);
        }

        Renderer& Renderer::drawRect(uint32_t name){
            assert(currentCommandBuffer != VK_NULL_HANDLE);

            Rect& rect = getRect(name);
            rect.bind(
                currentCommandBuffer,
                constantDescSets[gfx.frameIndex()],
                layouts.plRect,
                gfx.frameIndex()
            );
            pipelines.rect.bind(currentCommandBuffer, 
                VK_PIPELINE_BIND_POINT_GRAPHICS
            );
            models.rect.bind(currentCommandBuffer);
            models.rect.draw(currentCommandBuffer);

            return *this;
        }
        Renderer& Renderer::drawSprite(uint32_t name){
            assert(currentCommandBuffer != VK_NULL_HANDLE);

            Sprite& sprite = getSprite(name);
            sprite.bind(
                currentCommandBuffer,
                constantDescSets[gfx.frameIndex()],
                layouts.plSprite,
                gfx.frameIndex()
            );
            pipelines.sprite.bind(currentCommandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS
            );
            models.rect.bind(currentCommandBuffer);
            models.rect.draw(currentCommandBuffer);

            return *this;
        }

        void Renderer::setVsync(bool vsync){
            gfx.setVsync(vsync);
        }

        glm::mat4 Renderer::getProjectionMatrix(){
            // temp begin
            auto winExtent = getWindowExtent(window);
            // temp end
            return glm::ortho(
                -float(winExtent.width /2),  float(winExtent.width /2),
                 float(winExtent.height/2), -float(winExtent.height/2),
                -MAX_ZINDEX,                 MAX_ZINDEX
            );
        }
        void Renderer::cleanup(){
            assert(currentCommandBuffer == VK_NULL_HANDLE);
            gfx.device().waitIdle();
            vkDestroyPipelineLayout(
                gfx.device().device(), layouts.plSprite,
                nullptr
            );
            vkDestroyPipelineLayout(
                gfx.device().device(), layouts.plRect,
                nullptr
            );
        }

        VkPipelineLayout Renderer::createPipelineLayout(
            const std::vector<VkDescriptorSetLayout>& layouts
        ){
            VkPipelineLayout pLayout;

            VkPipelineLayoutCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            createInfo.setLayoutCount =  uint32_t(layouts.size());
            createInfo.pSetLayouts = layouts.data();

            shard_abort_ifnot(
                vkCreatePipelineLayout(
                    gfx.device().device(),
                    &createInfo, nullptr,
                    &pLayout
                )
                == VK_SUCCESS
            );

            return pLayout;
        }

    } // namespace r2d
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