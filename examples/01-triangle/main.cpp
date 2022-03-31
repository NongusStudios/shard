#include <shard/gfx/gfx.hpp>
#include <shard/time/time.hpp>

struct Vertex{
    glm::vec2 pos;
    glm::vec4 color;
};

Vertex vertices[] = {
    { { 0.0f, -0.5f }, {1.0f, 0.0f, 0.0f, 1.0f} },
    { { 0.5f,  0.5f }, {0.0f, 1.0f, 0.0f, 1.0f} },
    { {-0.5f,  0.5f }, {0.0f, 0.0f, 1.0f, 1.0f} },

};

int main(){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "02-cube", NULL, NULL);
    
    // Describes a binding description for the Vertex Shader
    VkVertexInputBindingDescription bindingDesc = {};
    bindingDesc.binding   = 0;
    bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    bindingDesc.stride    = sizeof(Vertex);

    // Vertex Attrib for the two input variables in tri.vert
    std::vector<VkVertexInputAttributeDescription> vertexAttrib(2);
    vertexAttrib[0].binding  = 0;
    vertexAttrib[0].format   = VK_FORMAT_R32G32_SFLOAT;
    vertexAttrib[0].location = 0;
    vertexAttrib[0].offset   = offsetof(Vertex, pos);

    vertexAttrib[1].binding  = 0;
    vertexAttrib[1].format   = VK_FORMAT_R32G32B32A32_SFLOAT;
    vertexAttrib[1].location = 1;
    vertexAttrib[1].offset   = offsetof(Vertex, color);

                                     // vsync
    shard::gfx::Graphics gfx(window, true);
    shard::gfx::Buffer   vertexBuffer = gfx.createVertexBuffer(sizeof(vertices), vertices);

    shard::gfx::Pipeline pipeline = gfx.createPipeline(
        gfx.emptyPipelineLayout(),
        "examples/01-triangle/tri.vert.spv", "examples/01-triangle/tri.frag.spv",
        {bindingDesc}, vertexAttrib,
        gfx.deafultPipelineConfig()
    );

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        // gfx.beginRenderPass() can return VK_NULL_HANDLE, if a window resize occurs
        // The first command is any Vulkan commands that you want to run before the render pass starts
        // For example memory barriers
        // The 2nd argument is the clear color in this case {44.0f, 44.0f, 44.0f, 255.0f}
        if(auto commandBuffer = gfx.beginRenderPass(nullptr, {44.0f})){
            pipeline.bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
            vertexBuffer.bindVertex(commandBuffer);
            vkCmdDraw(commandBuffer, 3, 1, 0, 0);
            gfx.endRenderPass();
        }
    }
    gfx.device().waitIdle();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}