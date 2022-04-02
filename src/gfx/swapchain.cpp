#include <shard/gfx/swapchain.hpp>

#include <limits>
#include <set>

namespace shard{
    namespace gfx{
        Swapchain::Swapchain(Device& refDevice, VkExtent2D winExtent, bool vsync):
            VSYNC{vsync},
            device{refDevice},
            windowExtent{windowExtent}
        {
            init();
        }
        Swapchain::Swapchain(
            Device& refDevice, VkExtent2D winExtent, bool vsync, std::shared_ptr<Swapchain> previous
        ):
            VSYNC{vsync},
            device{refDevice},
            windowExtent{windowExtent},
            oldSwapchain{previous}
        {
            init();
            oldSwapchain = nullptr;
        }
        Swapchain::~Swapchain(){
            for (auto imageView : swapchainImageViews) {
                vkDestroyImageView(device.device(), imageView, nullptr);
            }
            swapchainImageViews.clear();

            if (swapchain != nullptr) {
                vkDestroySwapchainKHR(device.device(), swapchain, nullptr);
                swapchain = nullptr;
            }

            vkDestroyRenderPass(device.device(), _renderPass, nullptr);

            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                vkDestroySemaphore(device.device(), renderFinishedSemaphores[i], nullptr);
                vkDestroySemaphore(device.device(), imageAvailableSemaphores[i], nullptr);
                vkDestroyFence(device.device(), inFlightFences[i], nullptr);
            }
        }

        void Swapchain::init(){
            createSwapchain();
            createImageViews();
            createRenderPass();
            createDepthResources();
            createFramebuffers();
            createSyncObjects();
        }
        void Swapchain::createSwapchain(){
            SwapchainSupportDetails swapchainSupport = device.getSwapchainSupportDetails();

            VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
            VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupport.presentModes);
            VkExtent2D extent = chooseSwapExtent(swapchainSupport.capabilities);

            uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
            if (swapchainSupport.capabilities.maxImageCount > 0 &&
                imageCount > swapchainSupport.capabilities.maxImageCount) {
                imageCount = swapchainSupport.capabilities.maxImageCount;
            }

            VkSwapchainCreateInfoKHR createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            createInfo.surface = device.surface();

            createInfo.minImageCount = imageCount;
            createInfo.imageFormat = surfaceFormat.format;
            createInfo.imageColorSpace = surfaceFormat.colorSpace;
            createInfo.imageExtent = extent;
            createInfo.imageArrayLayers = 1;
            createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            QueueFamilyIndices indices = device.getQueueFamilyIndices();
            std::set<uint32_t> queueFamilyIndexSet = {
                indices.graphics.value(),
                indices.present.value()
            };
            if(queueFamilyIndexSet.size() > 1){
                uint32_t queueFamilyIndices[] = {
                    indices.graphics.value(),
                    indices.present.value()
                };
                createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices = queueFamilyIndices;
            } else {
                createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                createInfo.queueFamilyIndexCount = 0;      // Optional
                createInfo.pQueueFamilyIndices = nullptr;  // Optional
            }

            createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
            createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

            createInfo.presentMode = presentMode;
            createInfo.clipped = VK_TRUE;

            createInfo.oldSwapchain = oldSwapchain == nullptr ? VK_NULL_HANDLE : oldSwapchain->swapchain;

            shard_abort_ifnot(
                vkCreateSwapchainKHR(device.device(), &createInfo, nullptr, &swapchain)
                == VK_SUCCESS
            );

            vkGetSwapchainImagesKHR(device.device(), swapchain, &imageCount, nullptr);
            swapchainImages.resize(imageCount);
            vkGetSwapchainImagesKHR(device.device(), swapchain, &imageCount, swapchainImages.data());

            _swapchainImageFormat = surfaceFormat.format;
            _swapchainExtent = extent;
        }
        void Swapchain::createImageViews(){
            swapchainImageViews.resize(swapchainImages.size());
            for(size_t i = 0; i < swapchainImages.size(); i++){
                VkImageViewCreateInfo viewInfo{};
                viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                viewInfo.image = swapchainImages[i];
                viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                viewInfo.format = _swapchainImageFormat;
                viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                viewInfo.subresourceRange.baseMipLevel = 0;
                viewInfo.subresourceRange.levelCount = 1;
                viewInfo.subresourceRange.baseArrayLayer = 0;
                viewInfo.subresourceRange.layerCount = 1;

                shard_abort_ifnot(
                    vkCreateImageView(device.device(), &viewInfo, nullptr, &swapchainImageViews[i])
                    == VK_SUCCESS
                );
            }
        }
        void Swapchain::createRenderPass(){
            VkAttachmentDescription depthAttachment{};
            depthAttachment.format = findDepthFormat();
            depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkAttachmentReference depthAttachmentRef{};
            depthAttachmentRef.attachment = 1;
            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkAttachmentDescription colorAttachment = {};
            colorAttachment.format = swapchainImageFormat();
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            VkAttachmentReference colorAttachmentRef = {};
            colorAttachmentRef.attachment = 0;
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass = {};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorAttachmentRef;
            subpass.pDepthStencilAttachment = &depthAttachmentRef;

            VkSubpassDependency dependency = {};
            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.srcAccessMask = 0;
            dependency.srcStageMask =
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.dstSubpass = 0;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            
            std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
            VkRenderPassCreateInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            renderPassInfo.pAttachments = attachments.data();
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpass;
            renderPassInfo.dependencyCount = 1;
            renderPassInfo.pDependencies = &dependency;

            shard_abort_ifnot(
                vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &_renderPass)
                == VK_SUCCESS
            );
        }
        void Swapchain::createDepthResources(){
            VkFormat depthFormat = findDepthFormat();
            swapchainDepthFormat = depthFormat;
            VkExtent2D swapChainExtent = swapchainExtent();

            for(size_t i = 0; i < imageCount(); i++){
                depthImages.push_back(Image(
                    device, swapChainExtent.width, swapChainExtent.height,
                    1, 0, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                    VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    0, VMA_MEMORY_USAGE_GPU_ONLY,
                    VK_IMAGE_ASPECT_DEPTH_BIT,
                    VK_SHARING_MODE_EXCLUSIVE
                ));
            }
        }
        void Swapchain::createFramebuffers(){
            for (size_t i = 0; i < imageCount(); i++) {
                std::vector<VkImageView> attachments = {swapchainImageViews[i], depthImages[i].imageView()};

                swapchainFramebuffers.push_back(Framebuffer(
                    device, _renderPass,
                    attachments, swapchainExtent()
                ));
            }
        }
        void Swapchain::createSyncObjects(){
            imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
            renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
            inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
            imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

            VkSemaphoreCreateInfo semaphoreInfo = {};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            VkFenceCreateInfo fenceInfo = {};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
                shard_abort_ifnot(
                    vkCreateSemaphore(
                        device.device(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i])
                        == VK_SUCCESS
                );
                shard_abort_ifnot(
                    vkCreateSemaphore(
                        device.device(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i])
                        == VK_SUCCESS
                );
                shard_abort_ifnot(
                    vkCreateFence(
                        device.device(), &fenceInfo, nullptr, &inFlightFences[i])
                        == VK_SUCCESS
                );
            }
        }

        VkSurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR> &availableFormats
        ){
        for (const auto &availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
            }
        }

        return availableFormats[0];
        }

        VkPresentModeKHR Swapchain::chooseSwapPresentMode(
            const std::vector<VkPresentModeKHR> &availablePresentModes
        ){
            if(VSYNC){
                for (const auto &availablePresentMode : availablePresentModes) {
                    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                        //std::cout << "Present mode: Mailbox" << std::endl;
                        return availablePresentMode;
                    }
                }
            } else {
                for (const auto &availablePresentMode : availablePresentModes) {
                    if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                        //std::cout << "Present mode: Immediate" << std::endl;
                        return availablePresentMode;
                    }
                }
            }
            return VK_PRESENT_MODE_FIFO_KHR;
        }

        VkExtent2D Swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
            if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()){
                return capabilities.currentExtent;
            } else {
                VkExtent2D actualExtent = windowExtent;
                actualExtent.width = std::max(
                    capabilities.minImageExtent.width,
                    std::min(capabilities.maxImageExtent.width, actualExtent.width)
                );
                actualExtent.height = std::max(
                    capabilities.minImageExtent.height,
                    std::min(capabilities.maxImageExtent.height, actualExtent.height)
                );

                return actualExtent;
            }
        }

        VkFormat Swapchain::findDepthFormat() {
            return device.findSupportedFormat(
                {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        } 

        VkResult Swapchain::acquireNextImage(uint32_t *imageIndex){
            vkWaitForFences(
                device.device(),
                1,
                &inFlightFences[currentFrame],
                VK_TRUE,
                std::numeric_limits<uint64_t>::max()
            );

            VkResult result = vkAcquireNextImageKHR(
                device.device(),
                swapchain,
                std::numeric_limits<uint64_t>::max(),
                imageAvailableSemaphores[currentFrame],  // must be a not signaled semaphore
                VK_NULL_HANDLE,
                imageIndex
            );

            return result;
        }
        VkResult Swapchain::submitCommandBuffers(
            const VkCommandBuffer *buffers, uint32_t *imageIndex
        ){
            if(imagesInFlight[*imageIndex] != VK_NULL_HANDLE){
                vkWaitForFences(device.device(), 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
            }
            imagesInFlight[*imageIndex] = inFlightFences[currentFrame];

            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
            VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = waitSemaphores;
            submitInfo.pWaitDstStageMask = waitStages;

            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = buffers;

            VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = signalSemaphores;

            vkResetFences(device.device(), 1, &inFlightFences[currentFrame]);
            shard_abort_ifnot(
                vkQueueSubmit(device.graphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame])
                == VK_SUCCESS
            );

            VkPresentInfoKHR presentInfo = {};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = signalSemaphores;

            VkSwapchainKHR swapchains[] = {swapchain};
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = swapchains;

            presentInfo.pImageIndices = imageIndex;

            auto result = vkQueuePresentKHR(device.presentQueue(), &presentInfo);

            currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

            return result;
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