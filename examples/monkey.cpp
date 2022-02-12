#include <shard/all.hpp>
#include <memory.h>
#include <unordered_map>
#include <functional>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

struct MonkeyVertex{
    glm::vec3 pos;
    glm::vec2 uv;
    glm::vec3 normal;
    glm::vec3 color;
    
    bool operator == (const MonkeyVertex& v) const {
        return pos == v.pos && uv == v.uv && normal == v.normal && color == v.color;
    }

    static VkVertexInputBindingDescription bindingDesc(){
        VkVertexInputBindingDescription binding = {};
        binding.binding = 0;
        binding.stride = sizeof(MonkeyVertex);
        binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return binding;
    }

    static std::vector<VkVertexInputAttributeDescription> attributeDescs(){
        std::vector<VkVertexInputAttributeDescription> attrs(4);

        attrs[0].binding = 0;
        attrs[0].location = 0;
        attrs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attrs[0].offset = offsetof(MonkeyVertex, pos);
        
        attrs[1].binding = 0;
        attrs[1].location = 1;
        attrs[1].format = VK_FORMAT_R32G32_SFLOAT;
        attrs[1].offset = offsetof(MonkeyVertex, uv);

        attrs[2].binding = 0;
        attrs[2].location = 2;
        attrs[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attrs[2].offset = offsetof(MonkeyVertex, normal);

        attrs[3].binding = 0;
        attrs[3].location = 3;
        attrs[3].format = VK_FORMAT_R32G32B32_SFLOAT;
        attrs[3].offset = offsetof(MonkeyVertex, color);

        return attrs;
    }
};

namespace std {
    template <>
    struct hash<MonkeyVertex> {
        size_t operator()(MonkeyVertex const &vertex) const {
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

struct Model{
    std::vector<MonkeyVertex> vertices;
    std::vector<uint32_t> indices;

    Model(const char* filePath){
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        shard_abort_ifnot(tinyobj::LoadObj(
                &attrib, &shapes, &materials, &warn, &err, filePath
        ));

        vertices.clear();
        indices.clear();

        std::unordered_map<MonkeyVertex, uint32_t> uniqueVertices = {};
        for (const auto &shape : shapes) {
            for (const auto &index : shape.mesh.indices) {
            MonkeyVertex vertex{};

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

class Monkey{
    public:
        Monkey():
            window{createWindow(800, 600)},
            input{window},
            gfx{window},
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
            pipeline{
                gfx.createPipeline(
                    pLayout,
                    "examples/monkey.vert.spv",
                    "examples/monkey.frag.spv",
                    {MonkeyVertex::bindingDesc()},
                    MonkeyVertex::attributeDescs(),
                    gfx.deafultPipelineConfig()
                )
            },
            monkey{"examples/monkey.obj"},
            vBuffer{
                gfx.createVertexBuffer(
                    monkey.vertices.size()*sizeof(MonkeyVertex),
                    monkey.vertices.data()
                )
            },
            iBuffer{
                gfx.createIndexBuffer(
                    monkey.indices.size()*sizeof(uint32_t),
                    monkey.indices.data()
                )
            }
        {
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
            if(VkCommandBuffer commandBuffer = gfx.beginRenderPass()){
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
                vBuffer.bindVertex(commandBuffer);
                iBuffer.bindIndex(commandBuffer, VK_INDEX_TYPE_UINT32);
                vkCmdDrawIndexed(
                    commandBuffer, monkey.indices.size(),
                    1, 0, 0, 0
                );
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
        shard::gfx::Pipeline pipeline;
        Model monkey;
        shard::gfx::Buffer vBuffer;
        shard::gfx::Buffer iBuffer;
        std::vector<shard::gfx::Buffer> uBuffers;
        std::vector<VkDescriptorSet> descSets;
        
        float rot = 0.0f;

        struct{
            glm::mat4 proj;
            glm::mat4 view;
            glm::mat4 model;
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