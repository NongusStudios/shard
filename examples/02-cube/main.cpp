#include <shard/gfx/gfx.hpp>
#include <shard/time/time.hpp>

struct Vertex{
    glm::vec3 pos;
    glm::vec4 color;
};

Vertex vertices[] = {
    { {-1.0f, -1.0f,  1.0f}, {1.0f, 0.0f, 0.0f, 1.0f} },
    { { 1.0f, -1.0f,  1.0f}, {1.0f, 0.0f, 0.0f, 1.0f} },
    { { 1.0f,  1.0f,  1.0f}, {1.0f, 0.0f, 0.0f, 1.0f} },
    { {-1.0f,  1.0f,  1.0f}, {1.0f, 0.0f, 0.0f, 1.0f} },

    { {-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f, 0.0f, 1.0f} },
    { { 1.0f, -1.0f, -1.0f}, {0.0f, 1.0f, 0.0f, 1.0f} },
    { { 1.0f,  1.0f, -1.0f}, {0.0f, 1.0f, 0.0f, 1.0f} },
    { {-1.0f,  1.0f, -1.0f}, {0.0f, 1.0f, 0.0f, 1.0f} }
};
uint32_t indices[] = {
    0, 1, 3, 3, 1, 2,
    1, 5, 2, 2, 5, 6,
    5, 4, 6, 6, 4, 7,
    4, 0, 7, 7, 0, 3,
    3, 2, 7, 7, 2, 6,
    4, 5, 0, 0, 5, 1
};

struct TransformData{
    glm::mat4 proj  = glm::mat4(1.0f);
    glm::mat4 model = glm::mat4(1.0f);
};

int main(){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "02-cube", NULL, NULL);
    
    VkVertexInputBindingDescription bindingDesc = {};
    bindingDesc.binding   = 0;
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    bindingDesc.stride    = sizeof(Vertex);

    std::vector<VkVertexInputAttributeDescription> vertexAttrib(2);
    vertexAttrib[0].binding  = 0;
    vertexAttrib[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
    vertexAttrib[0].location = 0;
    vertexAttrib[0].offset   = offsetof(Vertex, pos);

    vertexAttrib[1].binding  = 0;
    vertexAttrib[1].format   = VK_FORMAT_R32G32B32A32_SFLOAT;
    vertexAttrib[1].location = 1;
    vertexAttrib[1].offset   = offsetof(Vertex, color);
                                     // vsync
    shard::gfx::Graphics gfx(window, true);
    shard::gfx::Buffer   vertexBuffer = gfx.createVertexBuffer(sizeof(vertices), vertices);
    shard::gfx::Buffer   indexBuffer  = gfx.createIndexBuffer( sizeof(indices),  indices);

    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.size = sizeof(TransformData);

    auto pLayout = gfx.createPipelineLayout({pushConstantRange}, {});

    shard::gfx::Pipeline pipeline = gfx.createPipeline(
        pLayout,
        "examples/02-cube/cube.vert.spv", "examples/02-cube/cube.frag.spv",
        {bindingDesc}, vertexAttrib,
        gfx.deafultPipelineConfig()
    );

    TransformData tData = {};
    shard::Time time = {};

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        shard::time::updateTime(time);

        if(auto commandBuffer = gfx.beginRenderPass({44.0f})){
            VkExtent2D windowExtent = shard::getWindowExtent(window);
            tData.proj = glm::perspective(
                glm::radians(45.0f),
                float(windowExtent.width)/float(windowExtent.height),
                0.1f, 100.0f
            );
            tData.model = glm::mat4(1.0f);
            float rot = time.elapsed;
            glm::vec3 pos = {0.0f, 0.0f, -5.0f};
            glm::vec3 scale = {0.5f, 0.5f, 0.5f};
            tData.model = glm::translate(tData.model, pos);
            tData.model = glm::rotate(tData.model, rot, {0.5f, 1.0f, 0.5f});
            tData.model = glm::scale(tData.model, scale);

            vkCmdPushConstants(
                commandBuffer,
                pLayout,
                VK_SHADER_STAGE_VERTEX_BIT,
                0, sizeof(TransformData), &tData
            );

            pipeline.bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
            vertexBuffer.bindVertex(commandBuffer);
            indexBuffer.bindIndex(commandBuffer, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(commandBuffer, uint32_t(sizeof(indices)/sizeof(uint32_t)), 1, 0, 0, 0);
            gfx.endRenderPass();
        }
    }
    gfx.device().waitIdle();
    gfx.destroyPipelineLayout(pLayout);
}