#include <shard/gfx/gfx.hpp>

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <imgui.h>

void checkVkResult(VkResult result){
    shard_abort_ifnot(result == VK_SUCCESS);
}

int main(){
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "04-model", NULL, NULL);
    
    shard::gfx::Graphics gfx(window, false);
    shard::gfx::DescriptorPool descPool = gfx.createDescriptorPoolBuilder()
                                            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100)
                                            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100)
                                            .setMaxSets(1000).build();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = gfx.device().instance();
    initInfo.PhysicalDevice = gfx.device().pDevice();
    initInfo.Device = gfx.device().device();
    initInfo.QueueFamily = gfx.device().getQueueFamilyIndices().graphics.value();
    initInfo.Queue = gfx.device().graphicsQueue();
    initInfo.DescriptorPool = descPool.pool();
    initInfo.Subpass = 0;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = shard::gfx::Swapchain::MAX_FRAMES_IN_FLIGHT;
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    initInfo.Allocator = vmaGetAllocatorAllocationCallbacks(gfx.device().allocator());
    initInfo.CheckVkResultFn = checkVkResult;
    ImGui_ImplVulkan_Init(&initInfo, gfx.swapchain().renderPass());

    {
        auto cmd = gfx.device().beginSingleTimeCommands();
        ImGui_ImplVulkan_CreateFontsTexture(cmd);
        gfx.device().endSingleTimeCommands(cmd);
        gfx.device().waitIdle();
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();

        if(auto cmd = gfx.beginRenderPass({44.0f})){
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            if (show_demo_window)
                ImGui::ShowDemoWindow(&show_demo_window);
            
            {
                static float f = 0.0f;
                static int counter = 0;

                ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

                ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
                ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::Checkbox("Another Window", &show_another_window);

                ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
                ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

                if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                    counter++;
                ImGui::SameLine();
                ImGui::Text("counter = %d", counter);

                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::End();
            }
             if (show_another_window){
                ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
                ImGui::Text("Hello from another window!");
                if (ImGui::Button("Close Me"))
                    show_another_window = false;
                ImGui::End();
            }

            ImGui::Render();
            ImDrawData* draw_data = ImGui::GetDrawData();

            ImGui_ImplVulkan_RenderDrawData(draw_data, cmd);

            gfx.endRenderPass();
        }
    }

    gfx.device().waitIdle();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}