#include <shard/gfx/gfx.hpp>

#include <shard/imgui.hpp>

int main(){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "05-imgui", NULL, NULL);
    
    shard::gfx::Graphics gfx(window, false);
    shard::gfx::DescriptorPool descPool = gfx.createDescriptorPoolBuilder()
                                            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100)
                                            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100)
                                            .setMaxSets(1000).build();

    shard::imgui::init(window, gfx, descPool, VK_SAMPLE_COUNT_1_BIT);
    ImGui::StyleColorsDark();

    shard::gfx::Color clearColor = {44.0f};

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();

        if(auto cmd = gfx.beginRenderPass(clearColor)){
            shard::imgui::startFrame();
            
            static float ccolor[3] = {44.0f/255.0f, 44.0f/255.0f, 44.0f/255.0f};

            ImGui::Begin("Config");
            ImGui::ColorEdit3("clearColor", ccolor);
            clearColor = {ccolor[0]*255.0f, ccolor[1]*255.0f, ccolor[2]*255.0f};
            ImGui::End();
            
            shard::imgui::endFrame(cmd);

            gfx.endRenderPass();
        }
    }

    gfx.device().waitIdle();
    shard::imgui::terminate();

    glfwDestroyWindow(window);
    glfwTerminate();
}