#include <shard/renderer/renderer2d.hpp>

namespace shard{
    struct PushConstantData{
        glm::mat4 model;
        glm::mat4 view;  
    };

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
        const gfx::Vertex2D Circle::VERTICES[4] = {
            //            pos            
            gfx::Vertex2D({-1.0f, -1.0f}, {0.0f, 0.0f}, {1.0f}),
            gfx::Vertex2D({ 1.0f,  1.0f}, {1.0f, 1.0f}, {1.0f}),
            gfx::Vertex2D({-1.0f,  1.0f}, {0.0f, 1.0f}, {1.0f}),
            gfx::Vertex2D({ 1.0f, -1.0f}, {1.0f, 0.0f}, {1.0f})
        };
        const uint32_t      Circle::INDICES[6] = {
            0, 1, 2,
            0, 3, 1
        };

        Renderer::Renderer(
            GLFWwindow* win,
            const VkExtent2D& extent_,
            const uint32_t& texturePoolSize,
            const bool& vsync
        ):
            _window{win},
            _gfx{_window, vsync},
            _descPool{
                gfx::DescriptorPool::Builder(_gfx.device())
                    .addPoolSize(
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        gfx::Swapchain::MAX_FRAMES_IN_FLIGHT
                    )
                    .addPoolSize(
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        texturePoolSize
                    )
                    .setMaxSets(gfx::Swapchain::MAX_FRAMES_IN_FLIGHT)
                    .build()
            },
            _uniformDescLayout{
                gfx::DescriptorSetLayout::Builder(_gfx.device())
                    .addBinding(
                        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        VK_SHADER_STAGE_VERTEX_BIT
                    )
                    .build()
            },
            _texDescLayout{
                gfx::DescriptorSetLayout::Builder(_gfx.device())
                    .addBinding(
                        0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        VK_SHADER_STAGE_FRAGMENT_BIT
                    )
                    .build()
            },
            _tLoader{_gfx.device(), _descPool},
            _extent{extent_}
        {}
        Renderer::~Renderer(){}

        bool Renderer::beginRenderPass(const gfx::Color& color){
            if((currentCommandBuffer = _gfx.beginRenderPass(color)))
                return true;
            return false;
        }
        void Renderer::endRenderPass(){
            _gfx.endRenderPass();
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