#pragma once

#include <memory>

#include "../gfx/gfx.hpp"
#include "textureLoader.hpp"

namespace shard{
    namespace r2d{
        class Rect{
            public:
                static const gfx::Vertex2D VERTICES[4];
                static const uint32_t INDICES[6];

                Rect():
                    _pos{0.0f},
                    _rot{0.0f},
                    _scale{0.0f},
                    _color{}
                {}
                Rect(
                    const glm::vec2& pos_,
                    const float& rot_,
                    const glm::vec2& scale_,
                    const gfx::Color& color_
                ):
                    _pos{pos_},
                    _rot{rot_},
                    _scale{scale_},
                    _color{color_}
                {}

                Rect& setDeg(const float& deg){
                    _rot = glm::radians(deg);
                    return *this;
                }

                const glm::vec2&  pos()   const { return _pos;   }
                const float&      rot()   const { return _rot;   }
                const glm::vec2&  scale() const { return _scale; }
                const gfx::Color& color() const { return _color; }
                glm::vec2&        pos()         { return _pos;   }
                float&            rot()         { return _rot;   }
                glm::vec2&        scale()       { return _scale; }
                gfx::Color&       color()       { return _color; }
            private:
                glm::vec2 _pos;
                float _rot;
                glm::vec2 _scale;
                gfx::Color _color;
        };
        class Circle{
            public:
                static const gfx::Vertex2D VERTICES[4];
                static const uint32_t INDICES[6];

                Circle():
                    _pos{0.0f},
                    _radius{1.0f},
                    _color{}
                {}
                Circle(
                    const glm::vec2& pos_,
                    const float& radius_,
                    const gfx::Color& color_
                ):
                    _pos{pos_},
                    _radius{radius_},
                    _color{color_}
                {}

                const glm::vec2&  pos()    const { return _pos;    }
                const float&      radius() const { return _radius; }
                const gfx::Color& color()  const { return _color;  }
                glm::vec2&        pos()          { return _pos;    }
                float&            radius()       { return _radius; }
                gfx::Color&       color()        { return _color;  }
            private:
                glm::vec2 _pos;
                float _radius;
                gfx::Color _color;
        };

        class Renderer{
            public:
                class Builder{
                    public:
                        Builder(GLFWwindow* win):
                            window{win},
                            extent{getWindowExtent(window)},
                            texturePoolSize{0},
                            vsync{true}
                        {}
                        Builder& setExtent(const VkExtent2D& ext){
                            extent = ext;
                            return *this;
                        }
                        Builder& setTexturePoolSize(const uint32_t& texSz){
                            texturePoolSize = texSz;
                            return *this;
                        }
                        Builder& setVsync(const bool& _vsync){
                            vsync = _vsync;
                            return *this;
                        }
                        std::unique_ptr<Renderer> build(){
                            return std::make_unique<Renderer>(
                                window, extent, texturePoolSize, vsync
                            );
                        }
                    private:
                        GLFWwindow* window;
                        VkExtent2D extent;
                        uint32_t texturePoolSize;
                        bool vsync;
                };

                Renderer(
                    GLFWwindow* win,
                    const VkExtent2D& extent_,
                    const uint32_t& texturePoolSize,
                    const bool& vsync
                );
                ~Renderer();

                shard_delete_copy_constructors(Renderer);

                bool beginRenderPass(const gfx::Color& color);
                void endRenderPass();

                Renderer& drawRect(const Rect& rect);
                Renderer& drawRectWithBorder(
                    const Rect& rect, const gfx::Color& borderColor, const float& borderSize
                );

                Renderer& drawCircle(const Circle& circle);
                Renderer& drawCircleWithBorder(
                    const Circle& circle, const gfx::Color& borderColor, const float& borderSize
                );
                Renderer& drawSprite(
                    const Rect& srcRect, const Rect& rect, const uint32_t& texture
                );
                Renderer& drawSprite(const Circle& circle, const uint32_t& texture);

                void waitIdle(){
                    _gfx.device().waitIdle();
                }

                GLFWwindow* window(){
                    return _window;
                }
                gfx::Graphics& gfx(){
                    return _gfx;
                }
                gfx::DescriptorPool& descPool(){
                    return _descPool;
                }
                TextureLoader& tLoader(){
                    return _tLoader;
                }
                VkExtent2D extent() const {
                    return _extent;
                }
            private:
                GLFWwindow* _window;
                gfx::Graphics _gfx;
                gfx::DescriptorPool _descPool;
                gfx::DescriptorSetLayout _uniformDescLayout;
                gfx::DescriptorSetLayout _texDescLayout;
                TextureLoader _tLoader;

                VkExtent2D _extent;

                VkCommandBuffer currentCommandBuffer;
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
