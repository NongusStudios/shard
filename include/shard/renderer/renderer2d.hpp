#pragma once

#include <memory>
#include <map>

#include "../gfx/gfx.hpp"
#include "../gfx/model.hpp"

namespace shard{
    namespace r2d{
        class Rect{
            public:
                struct UBO {
                    alignas(16) glm::mat4 model;
                    alignas(16) gfx::Color color;
                    alignas(16) gfx::Color borderColor;
                    alignas(8)  glm::vec2 rectSize;
                    alignas(4)  VkBool32 hasBorder;
                    alignas(4)  float borderSize;
                };
                
                static const gfx::Vertex2D VERTICES[4];
                static const uint32_t INDICES[6];
                static gfx::Model createRectModel(gfx::Graphics& gfx);

                Rect(){}
                Rect(
                    gfx::Graphics& gfx,
                    gfx::DescriptorPool& pool,
                    gfx::DescriptorSetLayout& layout,
                    const uint32_t binding,
                    const glm::vec2& pos,
                    const float rot,
                    const glm::vec2& _scale,
                    const gfx::Color& _color,
                    const float _zindex,
                    const bool _hasBorder=false,
                    const float _borderSize=0.0f,
                    const gfx::Color& _borderColor={0.0f}
                );

                void bind(
                    VkCommandBuffer commandBuffer,
                    VkDescriptorSet constDescSet,
                    VkPipelineLayout pLayout,
                    uint32_t frameIndex
                );
                void rotationDegrees(float deg){
                    rotation = glm::radians(deg);
                }

                glm::mat4 modelMatrix() const;

                glm::vec2  position;
                float      rotation = 0.0f;
                glm::vec2  scale;
                gfx::Color color;
                float      zindex = 0.0f;
                bool       hasBorder = false;
                float      borderSize = false;
                gfx::Color borderColor;
            private:
                void cleanup(gfx::DescriptorPool& descPool);

                std::vector<gfx::Buffer> uBuffers;
                std::vector<VkDescriptorSet> _descSets;

                friend class Renderer;
        };
        class Sprite{
            public:
                struct UBO {
                    alignas(16) glm::mat4  model;
                    alignas(16) gfx::Color color;
                    alignas(8)  glm::vec2  srcRectPos;
                    alignas(8)  glm::vec2  srcRectSize;
                };
                
                Sprite(){}
                Sprite(
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
                );

                void bind(
                    VkCommandBuffer commandBuffer,
                    VkDescriptorSet constDescSet,
                    VkPipelineLayout pLayout,
                    uint32_t frameIndex
                );
                void rotationDegrees(float deg){
                    rotation = glm::radians(deg);
                }
                glm::mat4 modelMatrix() const;

                glm::vec2  position;
                float      rotation = 0.0f;
                glm::vec2  scale;
                gfx::Color color;
                float      zindex = 0.0f;
                struct{
                    glm::vec2 position;
                    glm::vec2 size;
                } srcRect;
            private:
                void cleanup(gfx::DescriptorPool& descPool);

                std::vector<gfx::Buffer> uBuffers;
                std::vector<VkDescriptorSet> _descSets;

                friend class Renderer;
        };

        class Renderer{
            public:
                struct UBO{
                    glm::mat4 projection;
                    glm::mat4 view;
                };
                struct Texture{
                    gfx::Image image;
                    gfx::Sampler sampler;
                };

                static constexpr uint32_t UBUFFER_POOL_SIZE = 500*2;
                static constexpr float MAX_ZINDEX = 100.0f;

                Renderer(
                    GLFWwindow* win,
                    VkExtent2D renderExtent,
                    uint32_t texturePoolSize,
                    bool _vsync
                );
                shard_delete_copy_constructors(Renderer);

                bool startFrame(const gfx::Color& color);
                void endFrame();

                uint32_t addRect(
                    const glm::vec2& position,
                    float rotation,
                    const glm::vec2& scale,
                    const gfx::Color& color,
                    float zindex
                );
                uint32_t addRect(
                    const glm::vec2& position,
                    float rotation,
                    const glm::vec2& scale,
                    const gfx::Color& color,
                    float zindex,
                    float borderSize,
                    const gfx::Color& borderColor
                );
                Rect& getRect(uint32_t name);
                void removeRect(uint32_t name);

                uint32_t addTexture(const char* filePath, VkFilter filter);
                Texture& getTexture(uint32_t name);
                void removeTexture(uint32_t name);

                uint32_t addSprite(
                    uint32_t texture,
                    const glm::vec2& position,
                    float rotation,
                    const glm::vec2& scale,
                    const gfx::Color& color,
                    float zindex
                );
                uint32_t addSprite(
                    uint32_t texture,
                    const glm::vec2& position,
                    float rotation,
                    const glm::vec2& scale,
                    const gfx::Color& color,
                    float zindex,
                    const glm::vec2& srcPos,
                    const glm::vec2& srcSize
                );
                Sprite& getSprite(uint32_t name);
                void removeSprite(uint32_t name);

                Renderer& drawRect(uint32_t name);
                Renderer& drawSprite(uint32_t name);
                
                gfx::Graphics& graphics(){
                    return gfx;
                }

                void setVsync(bool vsync);

                glm::mat4 getProjectionMatrix();
                void cleanup();
            private:
                VkPipelineLayout 
                createPipelineLayout(
                    const std::vector<VkDescriptorSetLayout>& layouts
                );

                GLFWwindow* window;
                gfx::Graphics gfx;
                gfx::DescriptorPool descPool;
                struct{
                    gfx::DescriptorSetLayout constant;
                    gfx::DescriptorSetLayout rect;
                    gfx::DescriptorSetLayout sprite;

                    VkPipelineLayout plRect;
                    VkPipelineLayout plSprite;
                } layouts;
                struct{
                    gfx::Pipeline rect;
                    gfx::Pipeline sprite;
                } pipelines;
                std::vector<gfx::Buffer> constantUniformBuffers;
                std::vector<VkDescriptorSet> constantDescSets;
                struct{
                    gfx::Model rect;
                } models;

                VkExtent2D extent;
                bool vsync;
                VkCommandBuffer currentCommandBuffer
                                = VK_NULL_HANDLE;
                struct{
                    uint32_t rect    = 1;
                    uint32_t texture = 1;
                    uint32_t sprite  = 1;
                } names;
                std::map<uint32_t, Rect> rects;
                std::map<uint32_t, std::unique_ptr<Texture>> textures;
                std::map<uint32_t, Sprite> sprites;
        };
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
