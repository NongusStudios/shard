#include <shard/imgui.hpp>

#include <shard/utils.hpp>

namespace shard{
    namespace imgui{
        void checkResult(VkResult result){
            shard_abort_ifnot(result == VK_SUCCESS);
        }
        ImGuiIO& init(
            GLFWwindow* window, gfx::Graphics& gfx, gfx::DescriptorPool& descPool,
            VkSampleCountFlagBits samples
        ){
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;

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
            initInfo.MSAASamples = samples;
            initInfo.Allocator = vmaGetAllocatorAllocationCallbacks(gfx.device().allocator());
            initInfo.CheckVkResultFn = shard::imgui::checkResult;
            ImGui_ImplVulkan_Init(&initInfo, gfx.swapchain().renderPass());

            auto cmd = gfx.device().beginSingleTimeCommands();
            ImGui_ImplVulkan_CreateFontsTexture(cmd);
            gfx.device().endSingleTimeCommands(cmd);
            gfx.device().waitIdle();
            ImGui_ImplVulkan_DestroyFontUploadObjects();

            return io;
        }
        void terminate(){
            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }

        void startFrame(){
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        }
        void endFrame(VkCommandBuffer cmd){
            assert(cmd != VK_NULL_HANDLE);
            ImGui::Render();
            ImDrawData* draw_data = ImGui::GetDrawData();
            ImGui_ImplVulkan_RenderDrawData(draw_data, cmd);
        }
    } // namespace imgui
} // namespace shard

/**
    Copyright 2022 Nongus Studios (https://github.com/NongusStudios/shard)
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/