#include <shard/gfx/gfx.hpp>

namespace shard{
    namespace gfx{
        Graphics::Graphics(GLFWwindow* win, bool vsync):
            VSYNC{vsync},
            _window{win},
            _defaultPipelineConfig{}
        {
            assert(_window != nullptr);

            _device = std::make_unique<Device>(_window);
            _swapchain = std::make_unique<Swapchain>(*_device, getFramebufferExtent(_window), VSYNC);
            _defaultPipelineConfig.makeDefault();
            createCommandBuffers();
            createEmptyPipelineLayout();
        }
        Graphics::~Graphics(){
            destroyCommandBuffers();
            vkDestroyPipelineLayout(_device->device(), _emptyPipelineLayout, nullptr);
        }

        void Graphics::recreateSwapchain(){
            VkExtent2D extent = getFramebufferExtent(_window);
            while(extent.width == 0 || extent.height == 0){
                extent = getFramebufferExtent(_window);
                glfwWaitEvents();
            }
            _device->waitIdle();

            std::shared_ptr<Swapchain> oldSwapchain = std::move(_swapchain);
            _swapchain = std::make_unique<Swapchain>(*_device, extent, VSYNC, oldSwapchain);
        }
        void Graphics::createCommandBuffers(){
            commandBuffers.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);

            VkCommandBufferAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = _device->commandPool();
            allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

            shard_abort_ifnot(
                vkAllocateCommandBuffers(
                    _device->device(), &allocInfo, commandBuffers.data()
                ) == VK_SUCCESS
            );
        }
        void Graphics::createEmptyPipelineLayout(){
            VkPipelineLayoutCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            shard_abort_ifnot(
                vkCreatePipelineLayout(
                    _device->device(), &createInfo, nullptr, &_emptyPipelineLayout
                ) == VK_SUCCESS
            );
        }
        
        void Graphics::destroyCommandBuffers(){
            vkFreeCommandBuffers(
                _device->device(),
                _device->commandPool(),
                static_cast<uint32_t>(commandBuffers.size()),
                commandBuffers.data()
            );
            commandBuffers.clear();
        }

        Pipeline Graphics::createPipeline(
            VkPipelineLayout layout,
            const char* vertFile,
            const char* fragFile,
            const std::vector<VkVertexInputBindingDescription>& bindingDescs,
            const std::vector<VkVertexInputAttributeDescription>& attrDescs,
            PipelineConfigInfo& config
        ){
            return Pipeline(device(), swapchain().renderPass(), layout, 
                vertFile, fragFile,
                bindingDescs, attrDescs,
                config
            );
        }
        Pipeline Graphics::createPipeline(
            VkPipelineLayout layout,
            const std::vector<char>& vertFile,
            const std::vector<char>& fragFile,
            const std::vector<VkVertexInputBindingDescription>& bindingDescs,
            const std::vector<VkVertexInputAttributeDescription>& attrDescs,
            PipelineConfigInfo& config
        ){
            return Pipeline(device(), swapchain().renderPass(), layout, 
                vertFile, fragFile,
                bindingDescs, attrDescs,
                config
            );
        }
        Buffer Graphics::createVertexBuffer(size_t size, const void* data){
            Buffer stagingBuffer = Buffer(
                device(),
                size, data,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VMA_MEMORY_USAGE_CPU_ONLY,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            );
            Buffer vBuf = Buffer(
                device(),
                size,
                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VMA_MEMORY_USAGE_GPU_ONLY,
                0
            );
            device().copyBuffer(stagingBuffer.buffer(), vBuf.buffer(), size);
            return vBuf;
        }
        Buffer Graphics::createIndexBuffer(size_t size, const void* data){
            Buffer stagingBuffer = Buffer(
                device(),
                size, data,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VMA_MEMORY_USAGE_CPU_ONLY,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            );
            Buffer iBuf = Buffer(
                device(),
                size,
                VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VMA_MEMORY_USAGE_GPU_ONLY,
                0
            );
            device().copyBuffer(stagingBuffer.buffer(), iBuf.buffer(), size);
            return iBuf;
        }
        Buffer Graphics::createUniformBuffer(size_t size, const void* data){
            return Buffer(
                device(), 
                size, data,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VMA_MEMORY_USAGE_CPU_TO_GPU,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            );
        }
        Image Graphics::createTexture(const char* filePath){
            return Image(*_device, filePath);
        }
        // Pixel format must be in VK_FORMAT_R8G8B8A8_BIT
        Image Graphics::createTexture(uint32_t w, uint32_t h, const void* pixels){
            return Image(*_device, w, h, pixels);
        }
        Image Graphics::createImage(
            uint32_t w, uint32_t h, uint32_t mipLevels,
            VkFormat format, VkImageTiling tiling,
            VkSampleCountFlagBits samples,
            VkImageUsageFlags usage,
            VkImageCreateFlags flags,
            VmaMemoryUsage memUsage,
            VkImageAspectFlags aspectMask
        ){
            return Image(*_device,
                w, h, mipLevels,
                0, format, tiling,
                samples, usage,
                flags, memUsage,
                aspectMask
            );
        }
        Framebuffer Graphics::createFramebuffer(std::vector<Image>& attachments){
            return Framebuffer(
                *_device, _swapchain->renderPass(), attachments
            );
        }
        Framebuffer Graphics::createFramebuffer(
            VkRenderPass renderPass, std::vector<Image>& attachments
        ){
            assert(renderPass != VK_NULL_HANDLE);
            return Framebuffer(
                *_device, renderPass, attachments
            );
        }
        Framebuffer Graphics::createFramebuffer(std::vector<Image>&& attachments){
            return Framebuffer(
                *_device, _swapchain->renderPass(), attachments
            );
        }
        Framebuffer Graphics::createFramebuffer(
            VkRenderPass renderPass, std::vector<Image>&& attachments
        ){
            assert(renderPass != VK_NULL_HANDLE);
            return Framebuffer(
                *_device, renderPass, attachments
            );
        }

        VkCommandBuffer Graphics::beginRenderPass(const Color& clearColor){
            assert(!isFrameStarted);
            VkResult result = _swapchain->acquireNextImage(&imageIndex);
            if(result == VK_ERROR_OUT_OF_DATE_KHR){
                recreateSwapchain();
                return VK_NULL_HANDLE;
            }
            shard_abort_ifnot(
                (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) &&
                "Failed to acquire next swapchain image!"
            );

            isFrameStarted = true;

            VkCommandBuffer commandBuffer = currentCommandBuffer();
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            shard_abort_ifnot(
                vkBeginCommandBuffer(
                    commandBuffer,
                    &beginInfo
                ) == VK_SUCCESS
            );

            VkRenderPassBeginInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = _swapchain->renderPass();
            renderPassInfo.framebuffer = _swapchain->getFrameBuffer(imageIndex).framebuffer();
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = _swapchain->swapchainExtent();

            std::array<VkClearValue, 2> clearValues;
            clearValues[0].color = {
                clearColor.r/255.0f,
                clearColor.g/255.0f,
                clearColor.b/255.0f,
                1.0f
            };
            clearValues[1].depthStencil = {1.0f, 0};
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(_swapchain->swapchainExtent().width);
            viewport.height = static_cast<float>(_swapchain->swapchainExtent().height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            VkRect2D scissor{{0, 0}, _swapchain->swapchainExtent()};
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            return commandBuffer;
        }
        void Graphics::endRenderPass(){
            assert(isFrameStarted && "Can't call endRenderPass while a frame is not in progress!");

            VkCommandBuffer commandBuffer = currentCommandBuffer();
            vkCmdEndRenderPass(commandBuffer);

            shard_abort_ifnot(
                vkEndCommandBuffer(commandBuffer) == VK_SUCCESS
            );

            VkResult result = _swapchain->submitCommandBuffers(&commandBuffer, &imageIndex);
            if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR){
                recreateSwapchain();
                result = VK_SUCCESS;
            }

            shard_abort_ifnot(
                result == VK_SUCCESS && "Failed to submit command buffers!"
            );

            isFrameStarted = false;
            currentFrameIndex = (currentFrameIndex + 1) % Swapchain::MAX_FRAMES_IN_FLIGHT;
        }
    } // namespace gfx
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