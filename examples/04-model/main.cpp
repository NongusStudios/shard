#include <shard/gfx/gfx.hpp>
#include <shard/gfx/model.hpp>
#include <shard/time/time.hpp>

struct UBO{
    glm::mat4 proj;
    glm::mat4 model;
};

int main(){
    shard::Time time = {};
    shard::gfx::ModelLoader loader("examples/04-model/model.obj");

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "04-model", NULL, NULL);
    
                                     // vsync
    shard::gfx::Graphics gfx(window, true);
    shard::gfx::Model    model(gfx, loader.vertices, loader.indices);

    auto descriptorPool = gfx.createDescriptorPoolBuilder().addPoolSize(
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, shard::gfx::Swapchain::MAX_FRAMES_IN_FLIGHT
    ).setMaxSets(shard::gfx::Swapchain::MAX_FRAMES_IN_FLIGHT).build();

    auto descriptorLayout = gfx.createDescriptorSetLayoutBuilder().addBinding(
        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT
    ).build();

    std::vector<shard::gfx::Buffer> uBuffers;
    std::vector<VkDescriptorSet> descSets(shard::gfx::Swapchain::MAX_FRAMES_IN_FLIGHT);
    size_t i = 0;
    for(auto& descSet : descSets){
        uBuffers.push_back(gfx.createUniformBuffer(sizeof(UBO), nullptr));
        uBuffers[i].map();
        auto descInfo = uBuffers[i].descriptorInfo();
        shard::gfx::DescriptorWriter(descriptorLayout, descriptorPool)
            .writeBuffer(0, &descInfo)
            .build(descSet);
        i++;
    }

    shard::gfx::PipelineConfigInfo config = {};
    config.makeDefault();
    config.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;

    auto pipelineLayout = gfx.createPipelineLayout({}, {&descriptorLayout});
    auto pipeline = gfx.createPipeline(
        pipelineLayout,
        "examples/04-model/model.vert.spv", "examples/04-model/model.frag.spv",
        {shard::gfx::Vertex3D::bindingDesc(VK_VERTEX_INPUT_RATE_VERTEX)}, shard::gfx::Vertex3D::attributeDescs(),
        config
    );

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        shard::time::updateTime(time);
        if(auto commandBuffer = gfx.beginRenderPass({44.0f})){
            VkExtent2D windowExtent = shard::getWindowExtent(window);
            UBO ubo = {};
            ubo.proj = glm::perspective(
                glm::radians(45.0f),
                float(windowExtent.width)/float(windowExtent.height),
                0.1f, 100.0f
            );
            ubo.model = glm::mat4(1.0f);
            float rot = time.elapsed;
            glm::vec3 pos = {0.0f, 0.0f, -5.0f};
            glm::vec3 scale = {1.0f, 1.0f, 1.0f};
            ubo.model = glm::translate(ubo.model, pos);
            ubo.model = glm::rotate(   ubo.model, rot, {0.0f, 1.0f, 0.0f});
            ubo.model = glm::scale(    ubo.model, scale);
            memcpy(
                uBuffers[gfx.frameIndex()].mappedMemory(), &ubo, sizeof(UBO)
            );
            vkCmdBindDescriptorSets(
                commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout, 0, 1, &descSets[gfx.frameIndex()],
                0, VK_NULL_HANDLE
            );
            pipeline.bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
            model.bind(commandBuffer);
            model.draw(commandBuffer);

            gfx.endRenderPass();
        }
    }

    gfx.device().waitIdle();
    gfx.destroyPipelineLayout(pipelineLayout);
}