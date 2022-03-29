#include <shard/gfx/gfx.hpp>

shard::gfx::Vertex2D vertices[] = {
    { { -1.0f, -1.0f }, { 0.0f, 0.0f }, {0.0f} },
    { {  1.0f, -1.0f }, { 1.0f, 0.0f }, {0.0f} },
    { { -1.0f,  1.0f }, { 0.0f, 1.0f }, {0.0f} },
    { {  1.0f,  1.0f }, { 1.0f, 1.0f }, {0.0f} },
};
uint32_t indices[] = {
    0, 1, 2,
    2, 1, 3
};

int main(){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "03-texture", NULL, NULL);
    
                                     // vsync
    shard::gfx::Graphics gfx(window, true);
    auto vertexBuffer = gfx.createVertexBuffer(sizeof(vertices), vertices);
    auto indexBuffer  = gfx.createIndexBuffer(sizeof(indices), indices);
    
    auto descriptorPool = gfx.createDescriptorPoolBuilder()
                             .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
                             .setMaxSets(1)
                             .build();
    auto descriptorLayout = gfx.createDescriptorSetLayoutBuilder()
                               .addBinding(
                                   0, 
                                   VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
                                   VK_SHADER_STAGE_FRAGMENT_BIT
                               ).build();
    

    auto image = gfx.createTexture("examples/03-texture/face.jpg");
    auto sampler = gfx.createSampler(
        VK_FILTER_LINEAR, VK_FILTER_LINEAR,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_SAMPLER_ADDRESS_MODE_REPEAT,
        VK_FALSE, VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        VK_SAMPLER_MIPMAP_MODE_LINEAR,
        image.mipMapLevels()
    );

    VkDescriptorSet descSet = VK_NULL_HANDLE;
    auto imageData = image.descriptorInfo(sampler);
    shard::gfx::DescriptorWriter(descriptorLayout, descriptorPool)
        .writeImage(0, &imageData)
        .build(descSet);
    
    auto pipelineLayout = gfx.createPipelineLayout({}, {&descriptorLayout});
    auto pipeline = gfx.createPipeline(
        pipelineLayout,
        "examples/03-texture/texture.vert.spv", "examples/03-texture/texture.frag.spv",
        {shard::gfx::Vertex2D::bindingDesc(VK_VERTEX_INPUT_RATE_VERTEX)}, shard::gfx::Vertex2D::attributeDescs(),
        gfx.deafultPipelineConfig()
    );

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();

        if(auto commandBuffer = gfx.beginRenderPass({44.0f})){
            vkCmdBindDescriptorSets(
                commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout, 0, 1, &descSet,
                0, VK_NULL_HANDLE
            );
            pipeline.bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
            vertexBuffer.bindVertex(commandBuffer);
            indexBuffer.bindIndex(commandBuffer, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);
            gfx.endRenderPass();
        }
    }
    gfx.device().waitIdle();
    gfx.destroyPipelineLayout(pipelineLayout);
}