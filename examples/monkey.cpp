#include <shard/all.hpp>
#include <memory.h>
#include <unordered_map>
#include <functional>
#include <ctime>

#define TINYOBJLOADER_IMPLEMENTATION
#include "lib/tiny_obj_loader.h"

namespace std {
    template <>
    struct hash<shard::gfx::Vertex3D> {
        size_t operator()(shard::gfx::Vertex3D const &vertex) const {
            size_t seed = 5381;
            uint8_t bytes[sizeof(vertex)];
            memcpy(bytes, &vertex, sizeof(bytes));
            for(size_t i = 0; i < sizeof(bytes); i++){
                seed = ((seed << 5) + seed) + bytes[i];
            } 
            return seed;
        }
    };
}  // namespace std

struct ModelLoader{
    std::vector<shard::gfx::Vertex3D> vertices;
    std::vector<uint32_t> indices;

    ModelLoader(const char* filePath){
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        shard_abort_ifnot(tinyobj::LoadObj(
                &attrib, &shapes, &materials, &warn, &err, filePath
        ));

        vertices.clear();
        indices.clear();

        std::unordered_map<shard::gfx::Vertex3D, uint32_t> uniqueVertices = {};
        for (const auto &shape : shapes) {
            for (const auto &index : shape.mesh.indices) {
            shard::gfx::Vertex3D vertex{};

            if (index.vertex_index >= 0) {
                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2],
                };

                vertex.color = {
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2],
                };
            }

            if (index.normal_index >= 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2],
                };
            }

            if (index.texcoord_index >= 0) {
                vertex.uv = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1],
                };
            }

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }
};

class MonkeyConfig{
    public:
        MonkeyConfig():
            config{}
        {
            config.makeDefault();
            config.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        }
        shard::gfx::PipelineConfigInfo config;
};

class Monkey{
    public:
        Monkey():
            window{createWindow(800, 600)},
            input{window},
            gfx{window, true},
            descPool{
                shard::gfx::DescriptorPool::Builder(gfx.device())
                    .setMaxSets(shard::gfx::Swapchain::MAX_FRAMES_IN_FLIGHT)
                    .addPoolSize(
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        shard::gfx::Swapchain::MAX_FRAMES_IN_FLIGHT
                    )
                    .build()
            },
            descLayout{
                shard::gfx::DescriptorSetLayout::Builder(gfx.device())
                    .addBinding(
                        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        VK_SHADER_STAGE_VERTEX_BIT
                    )
                    .build()
            },
            pLayout{
                createPipelineLayout()
            },
            config{},
            pipeline{
                gfx.createPipeline(
                    pLayout,
                    "examples/shaders/monkey.vert.spv",
                    "examples/shaders/monkey.frag.spv",
                    {shard::gfx::Vertex3D::bindingDesc()},
                    shard::gfx::Vertex3D::attributeDescs(),
                    config.config
                )
            },
            monkeyData{"examples/models/monkey.obj"},
            monkeyModel{
                gfx,
                monkeyData.vertices,
                monkeyData.indices
            }
        {
            std::srand((uint32_t)std::time(NULL));
            actions.exit = input.addAction(shard::Key::ESCAPE, 0);

            descSets.resize(shard::gfx::Swapchain::MAX_FRAMES_IN_FLIGHT);
            for(uint32_t i = 0; i < shard::gfx::Swapchain::MAX_FRAMES_IN_FLIGHT; i++){
                uBuffers.push_back(
                    gfx.createUniformBuffer(
                        sizeof(ubo), nullptr
                    )
                );
                uBuffers[i].map();

                auto uBufferInfo = uBuffers[i].descriptorInfo();
                shard::gfx::DescriptorWriter(descLayout, descPool)
                    .writeBuffer(0, &uBufferInfo)
                    .build(descSets[i]);
            }
        }
        ~Monkey(){
            glfwDestroyWindow(window);
        }

        void pInput(){
            switch (input.actionJustPressed(actions.exit)){
                case shard::input::Result::TRUE:
                    glfwSetWindowShouldClose(window, GLFW_TRUE);
                    break;
                default:
                    break;
            }
        }
        void update(){
            rot = time.elapsed;
        }
        void render(){
            if(VkCommandBuffer commandBuffer = gfx.beginRenderPass(
                shard::gfx::Color(44.0f, 44.0f, 44.0f)
            )){
                auto extent = shard::getWindowExtent(window);
                ubo.proj = glm::perspective(
                    glm::radians(45.0f), 
                    float(extent.width)/float(extent.height), 
                    0.1f, 100.0f
                );
                ubo.view = glm::mat4(1.0f);
                ubo.model = glm::mat4(1.0f);
                ubo.model = glm::translate(ubo.model, {0.0f, 0.0f, -5.0f});
                ubo.model = glm::rotate(ubo.model, rot, {0.0f, 1.0f, 0.0f});
                ubo.model = glm::scale(ubo.model, {1.0f, 1.0f, 1.0f});

                auto mPos = shard::getCursorPos(window);
                glm::vec3 normMousePos = glm::vec3(
                    mPos.x/float(extent.width),
                    mPos.y/float(extent.height),
                    std::clamp(
                        mPos.x/float(extent.width)*
                        mPos.y/float(extent.height),
                        0.0f, 1.0f
                    )
                );
                ubo.color = normMousePos;

                memcpy(
                    uBuffers[gfx.frameIndex()].mappedMemory(),
                    &ubo,
                    sizeof(ubo)
                );

                vkCmdBindDescriptorSets(
                    commandBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pLayout,
                    0, 1,
                    &descSets[gfx.frameIndex()],
                    0, nullptr
                );

                pipeline.bind(commandBuffer);
                monkeyModel.bind(commandBuffer);
                monkeyModel.draw(commandBuffer);

                gfx.endRenderPass();
            }
        }

        void run(){
            while(!glfwWindowShouldClose(window)){
                glfwPollEvents();
                shard::time::updateTime(time);

                pInput();
                update();
                render();
            }
            gfx.device().waitIdle();
            vkDestroyPipelineLayout(gfx.device().device(), pLayout, nullptr);
        }
    private:
        GLFWwindow* createWindow(int width, int height){
            shard_abort_ifnot(glfwInit());
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            //glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
            GLFWwindow* win = glfwCreateWindow(
                width, height,
                "App",
                nullptr, nullptr
            );
            return win;
        }
        VkPipelineLayout createPipelineLayout(){
            VkPipelineLayout layout = VK_NULL_HANDLE;

            VkDescriptorSetLayout descLayouts[] = {descLayout.layout()};

            VkPipelineLayoutCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            createInfo.setLayoutCount = 1;
            createInfo.pSetLayouts = descLayouts;

            shard_abort_ifnot(
                vkCreatePipelineLayout(
                    gfx.device().device(), &createInfo, nullptr, &layout
                ) == VK_SUCCESS
            );

            return layout;
        }

        GLFWwindow* window;
        shard::Input input;
        shard::gfx::Graphics gfx;
        shard::gfx::DescriptorPool descPool;
        shard::gfx::DescriptorSetLayout descLayout;
        VkPipelineLayout pLayout;
        MonkeyConfig config;
        shard::gfx::Pipeline pipeline;
        ModelLoader monkeyData;
        shard::gfx::Model monkeyModel;
        std::vector<shard::gfx::Buffer> uBuffers;
        std::vector<VkDescriptorSet> descSets;
        
        float rot = 0.0f;

        struct{
            glm::mat4 proj;
            glm::mat4 view;
            glm::mat4 model;
            glm::vec3 color = {1.0, 1.0, 1.0};
        } ubo;
        shard::Time time;

        struct {
            uint32_t exit;
        } actions;
};

int main(){
    std::unique_ptr<Monkey> monkey = std::make_unique<Monkey>();
    monkey->run();
    return 0;
}

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