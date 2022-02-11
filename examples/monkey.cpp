#include <shard/gfx/gfx.hpp>
#include <shard/time/time.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

using namespace shard::gfx;

struct MonkeyUBO{
    glm::mat4 proj = glm::perspective(90.0f, 800.0f/600.0f, 0.0f, 255.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 model = glm::mat4(1.0f);
};

struct MonkeyVertex{
    glm::vec3 pos;
    glm::vec2 uv;
    glm::vec3 normal;
    glm::vec3 color;

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
        attrs[3].offset = offsetof(MonkeyVertex, normal);

        return attrs;
    }
};

std::vector<MonkeyVertex> loadModel(const char* filepath){
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath)) {
        std::cerr << warn + err << "\n";
        std::abort();
    }

    std::vector<MonkeyVertex> vertices = {};
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
            vertices.push_back(vertex);
        }
    }
    return vertices;
}

class MonkeyConfig{
    public:
        PipelineConfigInfo config;
        MonkeyConfig(){
            config.makeDefault();
            config.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        }
};

class Monkey{
    public:
        Monkey():
            window{createWindow(800, 600)},
            gfx{window},
            descPool{
                DescriptorPool::Builder(gfx.device())
                    .setMaxSets(Swapchain::MAX_FRAMES_IN_FLIGHT)
                    .setPoolFlags(0)
                    .addPoolSize(
                        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        Swapchain::MAX_FRAMES_IN_FLIGHT
                    )
                    .build()
            },
            descLayout{
                DescriptorSetLayout::Builder(gfx.device())
                    .addBinding(
                        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        VK_SHADER_STAGE_VERTEX_BIT
                    )
                    .build()
            },
            config{},
            pLayout{
                createPipelineLayout()
            },
            monkeyPipeline{
                gfx.createPipeline(
                    pLayout,
                    "examples/monkey.vert.spv", "examples/monkey.frag.spv",
                    {MonkeyVertex::bindingDesc()}, MonkeyVertex::attributeDescs(),
                    config.config
                )
            },
            vertices{
                loadModel("examples/monkey.obj")
            },
            monkeyVertexBuffer{
                gfx.createVertexBuffer(
                    sizeof(MonkeyVertex)*vertices.size(),
                    vertices.data()
                )
            }
        {
            descSets.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);
            for(size_t i = 0; i < Swapchain::MAX_FRAMES_IN_FLIGHT; i++){
                uniformBuffers.push_back(
                    gfx.createUniformBuffer(
                        sizeof(MonkeyUBO), nullptr
                    )
                );
                uniformBuffers[i].map();
            }

            for(size_t i = 0; i < Swapchain::MAX_FRAMES_IN_FLIGHT; i++){
                auto uniformBufferInfo = uniformBuffers[i].descriptorInfo();
                DescriptorWriter(descLayout, descPool)
                    .writeBuffer(0, &uniformBufferInfo)
                    .build(descSets[i]);
            }

            run();
        }
        ~Monkey(){
            glfwDestroyWindow(window);
        }
        
        void update(){
            monkeyRot = time.elapsed;
        }
        void render(){
            if(auto commandBuffer = gfx.beginRenderPass()){
                monkeyUbo.model = glm::mat4(1.0f);
                monkeyUbo.model = glm::translate(monkeyUbo.model, monkeyPos);
                monkeyUbo.model = glm::rotate(monkeyUbo.model, monkeyRot, {0.0f, 1.0f, 0.0f});
                monkeyUbo.model = glm::scale(monkeyUbo.model, monkeyScale);

                memcpy(
                    uniformBuffers[gfx.frameIndex()].mappedMemory(), &monkeyUbo, sizeof(MonkeyUBO)
                );
                vkCmdBindDescriptorSets(
                    commandBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pLayout,
                    0, 1,
                    &descSets[gfx.frameIndex()],
                    0, nullptr
                );

                monkeyPipeline.bind(commandBuffer);
                monkeyVertexBuffer.bindVertex(commandBuffer);
                vkCmdDraw(commandBuffer, uint32_t(vertices.size()), 1, 0, 0);
                gfx.endRenderPass();
            }
        }

        void run(){
            while(!glfwWindowShouldClose(window)){
                glfwPollEvents();
                shard::time::updateTime(time);
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
        Graphics gfx;
        DescriptorPool descPool;
        DescriptorSetLayout descLayout;
        std::vector<VkDescriptorSet> descSets;
        MonkeyConfig config;
        VkPipelineLayout pLayout;
        Pipeline monkeyPipeline;
        std::vector<MonkeyVertex> vertices;
        Buffer monkeyVertexBuffer;
        std::vector<Buffer> uniformBuffers;

        shard::Time time;

        MonkeyUBO monkeyUbo;
        glm::vec3 monkeyPos = {0.0f, 0.0f, 10.0f};
        float monkeyRot = 0.0f;
        glm::vec3 monkeyScale = {2.0f, 2.0f, 2.0f};
};


int main(){
    std::unique_ptr<Monkey> monkey = std::make_unique<Monkey>();
    monkey->run();
}