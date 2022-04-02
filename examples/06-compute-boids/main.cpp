#include <shard/gfx/gfx.hpp>
#include <shard/time/time.hpp>
#include <shard/random/random.hpp>
#include <shard/imgui.hpp>

const uint32_t BOID_COUNT = 10000;
const int WINDOW_WIDTH  = 800;
const int WINDOW_HEIGHT = 600;

shard::gfx::Vertex2D boidVertices[] = {
    { { 0.0f, -0.5f}, {}, {} },
    { {-0.5f,  0.5f}, {}, {} },
    { { 0.5f,  0.5f}, {}, {} }
};

struct ComputeData{
    alignas(4)  float      deltaTime;
    alignas(4)  float      rotationSpeed;
    alignas(4)  float      boidSpeed;
    alignas(4)  float      boidSpeedVariation;
    alignas(8)  glm::vec2  flockPosition;
    alignas(4)  float      neighbourDistance;
    alignas(4)  int        boidCount;
    alignas(8)  glm::vec2  time;
};
struct VertexData{
    glm::mat4 projection;
};
struct Boid{
    glm::vec2    position;
    glm::vec2    direction;
    float        noiseOffset;
    glm::vec3    color;
};

VkSharingMode getComputeSharingMode(shard::gfx::Graphics& gfx){
    if(gfx.device().getQueueFamilyIndices().graphics.value() != gfx.device().getQueueFamilyIndices().compute.value())
        return VK_SHARING_MODE_CONCURRENT;
    return VK_SHARING_MODE_EXCLUSIVE;
}

int main(){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(
        WINDOW_WIDTH, WINDOW_HEIGHT, "06-compute-boids", nullptr, nullptr
    );

    shard::gfx::Graphics gfx(window, false);
    auto descPool = gfx.createDescriptorPoolBuilder().addPoolSize(
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10
    ).addPoolSize(
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10
    ).setMaxSets(1000).build();

    shard::Time time = {}; shard::time::updateTime(time);
    shard::randy::Random rng(time.ticks);

    auto vertexBuffer = gfx.createVertexBuffer(
        sizeof(boidVertices), VK_SHARING_MODE_EXCLUSIVE, boidVertices
    );

    auto boidDescSetLayout = gfx.createDescriptorSetLayoutBuilder().addBinding(
        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT
    ).addBinding(
        1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT
    ).build();
    auto boidLayout = gfx.createPipelineLayout({}, {&boidDescSetLayout});
    auto boidPipeline = gfx.createPipeline(
        boidLayout, "examples/06-compute-boids/boids.vert.spv", "examples/06-compute-boids/boids.frag.spv",
        {shard::gfx::Vertex2D::bindingDesc(VK_VERTEX_INPUT_RATE_VERTEX)},
        shard::gfx::Vertex2D::attributeDescs(), gfx.deafultPipelineConfig()
    );

    std::vector<Boid> boidData(BOID_COUNT);
    for(auto& boid : boidData){
        boid.color = {
            rng.randf(),
            rng.randf(),
            rng.randf()
        };
        boid.noiseOffset = rng.randf()*1000.0f;
    }
    
    auto boidBuffer = gfx.createStorageBuffer_GPUonly(
        sizeof(Boid)*BOID_COUNT, getComputeSharingMode(gfx), boidData.data()
    );

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(
        gfx.device().device(), boidBuffer.buffer(), &memReq
    );

    auto boidCompDescLayout = gfx.createDescriptorSetLayoutBuilder().addBinding(
        0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT
    ).addBinding(
        1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT
    ).build();
    auto boidCompLayout = gfx.createPipelineLayout({}, {&boidCompDescLayout});
    auto computePipeline = gfx.createCompute(
        boidCompLayout, "examples/06-compute-boids/boids.comp.spv"
    );

    auto computeUniformBuffer = gfx.createUniformBuffer(
        sizeof(ComputeData), VK_SHARING_MODE_EXCLUSIVE, nullptr
    );
    computeUniformBuffer.map();
    auto computeUniformBufferInfo = computeUniformBuffer.descriptorInfo();
    auto boidBufferInfo = boidBuffer.descriptorInfo();
    VkDescriptorSet computeDescSet = VK_NULL_HANDLE;
    shard::gfx::DescriptorWriter(
        boidCompDescLayout, descPool
    ).writeBuffer(0, &computeUniformBufferInfo).writeBuffer(1, &boidBufferInfo).build(computeDescSet);

    std::vector<shard::gfx::Buffer> uBuffers = {};
    std::vector<VkDescriptorSet> descSets(shard::gfx::Swapchain::MAX_FRAMES_IN_FLIGHT);
    uint32_t idx = 0;
    for(auto& descSet : descSets){
        uBuffers.push_back(
            gfx.createUniformBuffer(sizeof(VertexData), VK_SHARING_MODE_EXCLUSIVE, nullptr)
        );
        uBuffers[idx].map();
        auto uBufInfo = uBuffers[idx].descriptorInfo();
        shard::gfx::DescriptorWriter(boidDescSetLayout, descPool).writeBuffer(
            0, &uBufInfo
        ).writeBuffer(
            1, &boidBufferInfo
        ).build(descSet);
        idx++;
    }

    VkCommandBuffer computeCommands = gfx.allocateComputeCommandBuffer();

    shard::imgui::init(window, gfx, descPool, VK_SAMPLE_COUNT_1_BIT);
    
    ComputeData computeData = {};
    computeData.boidCount = BOID_COUNT;
    computeData.boidSpeed = 2000.0f;
    computeData.boidSpeedVariation = 0.1f;
    computeData.deltaTime = 0.0f;
    computeData.neighbourDistance = 120.0f;
    computeData.rotationSpeed = 10.0f;

    VertexData vertData = {};

    shard::time::addTimer(time, "fpsTimer", 1.0f, [&](shard::Timer& timer){
        std::cout << time.fps << "\n";
    });

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        shard::time::updateTime(time);

        gfx.beginComputeCommands(computeCommands);
        auto mousePos = shard::getCursorPos(window)*2.0f;
        computeData.flockPosition = mousePos;
        
        computeData.flockPosition = mousePos;

        computePipeline.bind(computeCommands);
        computeData.deltaTime = time.dt;
        computeData.time.x = time.elapsed / 20;
        computeData.time.y = time.elapsed;
        memcpy(
            computeUniformBuffer.mappedMemory(), &computeData, sizeof(ComputeData)
        );
        vkCmdBindDescriptorSets(
            computeCommands, VK_PIPELINE_BIND_POINT_COMPUTE,
            boidCompLayout, 0, 1, &computeDescSet,
            0, nullptr
        );
        computePipeline.dispatch(computeCommands, uint32_t((BOID_COUNT/256))+1, 1);
        gfx.submitComputeCommands(computeCommands);

        if(auto commands = gfx.beginRenderPass([&](VkCommandBuffer cmd){
            VkBufferMemoryBarrier barrier = {};
            barrier.sType  = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            barrier.buffer = boidBuffer.buffer();
            barrier.size   = boidBuffer.size();
            barrier.srcQueueFamilyIndex = gfx.device().getQueueFamilyIndices().compute.value();
            barrier.dstQueueFamilyIndex = gfx.device().getQueueFamilyIndices().graphics.value();
            barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(
                cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
                0, 0, nullptr, 1, &barrier, 0, nullptr
            );
        }, {44.0f})){
            boidPipeline.bind(commands, VK_PIPELINE_BIND_POINT_GRAPHICS);
            auto windowExtent = shard::getWindowExtent(window);
            vertData.projection = glm::ortho(
                0.0f, float(windowExtent.width*2), 0.0f, float(windowExtent.height*2), -100.0f, 100.0f
            );
            memcpy(
                uBuffers[gfx.frameIndex()].mappedMemory(), &vertData, sizeof(VertexData)
            );
            vkCmdBindDescriptorSets(
                commands, VK_PIPELINE_BIND_POINT_GRAPHICS,
                boidLayout, 0, 1, &descSets[gfx.frameIndex()],
                0, nullptr
            );
            vertexBuffer.bindVertex(commands);
            vkCmdDraw(commands, 3, BOID_COUNT, 0, 0);
            gfx.endRenderPass();
        }
    }

    gfx.device().waitIdle();
    gfx.destroyPipelineLayout(boidLayout);
    gfx.destroyPipelineLayout(boidCompLayout);
    gfx.freeComputeCommandBuffer(computeCommands);
    shard::imgui::terminate();
    glfwDestroyWindow(window);
    glfwTerminate();
}