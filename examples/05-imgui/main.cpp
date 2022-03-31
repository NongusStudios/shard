#include <shard/gfx/gfx.hpp>

#include <shard/imgui.hpp>

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
    GLFWwindow* window = glfwCreateWindow(800, 600, "05-imgui", NULL, NULL);
    
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

    shard::gfx::Graphics gfx(window, false);
    shard::gfx::DescriptorPool descPool = gfx.createDescriptorPoolBuilder()
                                            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100)
                                            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100)
                                            .setMaxSets(1000).build();
    shard::gfx::Buffer   vertexBuffer = gfx.createVertexBuffer(sizeof(vertices), vertices);
    shard::gfx::Buffer   indexBuffer  = gfx.createIndexBuffer( sizeof(indices),  indices);
    
    VkPushConstantRange pushConstantRange = {};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.size = sizeof(TransformData);
    VkPushConstantRange pushConstantRange2 = {};
    pushConstantRange2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange2.size = sizeof(shard::gfx::Color);

    auto pLayout = gfx.createPipelineLayout({pushConstantRange, pushConstantRange2}, {});

    shard::gfx::Pipeline pipeline = gfx.createPipeline(
        pLayout,
        "examples/02-cube/cube.vert.spv", "examples/02-cube/cube.frag.spv",
        {bindingDesc}, vertexAttrib,
        gfx.deafultPipelineConfig()
    );

    TransformData tData = {};

    shard::imgui::init(window, gfx, descPool, VK_SAMPLE_COUNT_1_BIT);

    shard::gfx::Color clearColor = {44.0f};

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();

        if(auto cmd = gfx.beginRenderPass(nullptr, clearColor)){
            shard::imgui::startFrame();
            
            static float ccolor[3] = {44.0f/255.0f, 44.0f/255.0f, 44.0f/255.0f};
            VkExtent2D windowExtent = shard::getWindowExtent(window);
            tData.proj = glm::perspective(
                glm::radians(45.0f),
                float(windowExtent.width)/float(windowExtent.height),
                0.1f, 100.0f
            );

            ImGui::Begin("Config");
            ImGui::ColorEdit3("clearColor", ccolor);
            clearColor = {ccolor[0]*255.0f, ccolor[1]*255.0f, ccolor[2]*255.0f};
            static glm::vec3 pos = {0.0f, 0.0f, -5.0f};
            static glm::vec3 scale = {0.5f, 0.5f, 0.5f};
            static glm::vec3 rot = {-0.6f, 0.2f, 0.0f};
            ImGui::SliderFloat3("pos", &pos[0], -10.0f, 10.0f);
            ImGui::SliderFloat3("rot", &rot[0], -glm::pi<float>()*2, glm::pi<float>()*2);
            ImGui::SliderFloat3("scale", &scale[0], -10.0f, 10.0f);
            
            if(ImGui::Button("Reset")){
                pos = {0.0f, 0.0f, -5.0f};
                scale = {0.5f, 0.5f, 0.5f};
                rot = {-0.6f, 0.2f, 0.0f};
            }
            if(ImGui::Button("Quit"))
                glfwSetWindowShouldClose(window, true);
            ImGui::End();
            
            tData.model = glm::mat4(1.0f);
            tData.model = glm::translate(tData.model, pos);
            tData.model = glm::rotate(tData.model, rot.x, {0.0f, 1.0f, 0.0f});
            tData.model = glm::rotate(tData.model, rot.y, {1.0f, 0.0f, 0.0f});
            tData.model = glm::rotate(tData.model, rot.z, {0.0f, 0.0f, 1.0f});
            tData.model = glm::scale(tData.model, scale);

            vkCmdPushConstants(
                cmd,
                pLayout,
                VK_SHADER_STAGE_VERTEX_BIT,
                0, sizeof(TransformData), &tData
            );

            pipeline.bind(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS);
            vertexBuffer.bindVertex(cmd);
            indexBuffer.bindIndex(cmd, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(cmd, uint32_t(sizeof(indices)/sizeof(uint32_t)), 1, 0, 0, 0);

            shard::imgui::endFrame(cmd);
            gfx.endRenderPass();
        }
    }

    gfx.device().waitIdle();
    gfx.destroyPipelineLayout(pLayout);
    shard::imgui::terminate();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}