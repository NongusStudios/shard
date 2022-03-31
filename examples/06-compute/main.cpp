#include <shard/gfx/gfx.hpp>
#include <shard/time/time.hpp>
#include <shard/random/random.hpp>
#include <shard/input/input.hpp>

struct Vertex{
    alignas(8)  glm::vec2 pos;
    alignas(16) glm::vec3 color;
};
struct ComputeUBO{
    alignas(4) int colorCount;
    alignas(4) float seed;
};

const uint32_t TRI_COUNT = 10;

shard::gfx::Buffer runCompute(
    shard::gfx::Graphics& gfx, VkCommandBuffer ccmd, shard::gfx::Buffer& storageBuffer,
    VkPipelineLayout computeLayout, VkDescriptorSet descSet,
    shard::gfx::Compute& compute
){
    gfx.beginComputeCommands(ccmd);
    compute.bind(ccmd);
    vkCmdBindDescriptorSets(
        ccmd, VK_PIPELINE_BIND_POINT_COMPUTE, computeLayout,
        0, 1, &descSet, 0, VK_NULL_HANDLE
    );
    compute.dispatch(ccmd, TRI_COUNT*3, 1);
    gfx.submitComputeCommands(ccmd);

    Vertex* vertData = (Vertex*)storageBuffer.mappedMemory();
    return gfx.createVertexBuffer(storageBuffer.size(), vertData);
}

int main(){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "06-compute", NULL, NULL);

    shard::Time time = {};
    shard::time::updateTime(time);
    shard::randy::Random rng(time.ticks);

    shard::gfx::Graphics gfx(window, true);
    auto descPool = gfx.createDescriptorPoolBuilder().addPoolSize(
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100
    ).addPoolSize(
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100
    ).setMaxSets(1000).build();

    auto computeCommandBuffer = gfx.allocateComputeCommandBuffer();
    
    std::vector<Vertex> vertices(TRI_COUNT*3);
    for(uint32_t i = 0; i < TRI_COUNT*3; i += 3){
        for(uint32_t j = 0; j < 3; j++){
            vertices[i+j] = {
                {
                    rng.randRangef(-1.0f, 0.5f),
                    rng.randRangef(-1.0f, 0.5f)
                },
                {}
            };
        }
    }

    auto storageBuffer = gfx.createStorageBuffer(sizeof(Vertex)*vertices.size(), vertices.data());
    storageBuffer.map();

    VkVertexInputBindingDescription bindingDesc = {};
    bindingDesc.binding   = 0;
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    bindingDesc.stride    = sizeof(Vertex);

    std::vector<VkVertexInputAttributeDescription> vertexAttrib(2);
    vertexAttrib[0].binding  = 0;
    vertexAttrib[0].format   = VK_FORMAT_R32G32_SFLOAT;
    vertexAttrib[0].location = 0;
    vertexAttrib[0].offset   = offsetof(Vertex, pos);

    vertexAttrib[1].binding  = 0;
    vertexAttrib[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
    vertexAttrib[1].location = 1;
    vertexAttrib[1].offset   = offsetof(Vertex, color);

    shard::gfx::Pipeline pipeline = gfx.createPipeline(
        gfx.emptyPipelineLayout(),
        "examples/06-compute/shader.vert.spv", "examples/06-compute/shader.frag.spv",
        {bindingDesc}, vertexAttrib,
        gfx.deafultPipelineConfig()
    );

    ComputeUBO ubo = {
        TRI_COUNT*3,
        (rng.randRangef(0.0f, 100.0f))*10000.0f
    };

    auto computeUniformBuffer = gfx.createUniformBuffer(sizeof(ComputeUBO), &ubo);
    computeUniformBuffer.map();

    auto setLayout = gfx.createDescriptorSetLayoutBuilder().addBinding(
        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT
    ).addBinding(
        1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT
    ).build();

    auto computeUniformBufferInfo = computeUniformBuffer.descriptorInfo();
    auto storageBufferInfo = storageBuffer.descriptorInfo();
    VkDescriptorSet descSet;
    shard::gfx::DescriptorWriter(setLayout, descPool).writeBuffer(
        0, &computeUniformBufferInfo
    ).writeBuffer(
        1, &storageBufferInfo
    ).build(descSet);

    VkPipelineLayout computeLayout = gfx.createPipelineLayout({}, {&setLayout});
    auto compute = gfx.createCompute(computeLayout, "examples/06-compute/shader.comp.spv");

    auto vertexBuffer = runCompute(gfx, computeCommandBuffer, storageBuffer, computeLayout, descSet, compute);

    shard::Input input(window);
    uint32_t regenColoursAction = input.addAction(shard::Key::R, 0);

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        
        
        if(input.actionJustPressed(regenColoursAction) == shard::input::Result::TRUE){
            ubo.seed = (rng.randRangef(0.0f, 100.0f))*10000.0f;
            memcpy(computeUniformBuffer.mappedMemory(), &ubo, sizeof(ubo));
            vertexBuffer = runCompute(gfx, computeCommandBuffer, storageBuffer, computeLayout, descSet, compute);
        }

        if(auto commandBuffer = gfx.beginRenderPass(
            // The function given will be run before the render pass begins
            [&](VkCommandBuffer cmd){
                VkBufferMemoryBarrier barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
                barrier.buffer = storageBuffer.buffer();
                barrier.size = storageBuffer.size();
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                barrier.srcQueueFamilyIndex = gfx.device().getQueueFamilyIndices().compute.value();
                barrier.dstQueueFamilyIndex = gfx.device().getQueueFamilyIndices().graphics.value();

                vkCmdPipelineBarrier(
                    cmd,
                    VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                    VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
                    0, 0, nullptr, 1, &barrier, 0, nullptr
                );
            }, {44.0f}
            )
        ){
            
            pipeline.bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
            vertexBuffer.bindVertex(commandBuffer);
            vkCmdDraw(commandBuffer, vertices.size(), 1, 0, 0);
            gfx.endRenderPass();
        }
    }

    gfx.device().waitIdle();
    gfx.destroyPipelineLayout(computeLayout);
    gfx.freeComputeCommandBuffer(computeCommandBuffer);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}