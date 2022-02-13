#pragma once

#include "../gfx/gfx.hpp"

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
    }

    class Renderer2D{
        public:
            Renderer2D(GLFWwindow* _window, VkExtent2D _renderRes);
            ~Renderer2D();

            bool begin(const gfx::Color& color);
            void present();
        private:
            GLFWwindow* window;
            VkExtent2D renderRes;
            gfx::Graphics gfx;
            gfx::Buffer rectVertexBuffer;
            gfx::Buffer rectIndexBuffer;

            VkCommandBuffer currentCommandBuffer;
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
