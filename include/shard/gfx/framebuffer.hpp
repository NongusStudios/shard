#pragma once

#include "device.hpp"
#include "image.hpp"

namespace shard{
    namespace gfx{
        class Framebuffer{
            public:
                Framebuffer(Device& _device):
                    device{_device},
                    _framebuffer{VK_NULL_HANDLE},
                    _extent{0, 0}
                {}
                Framebuffer(
                    Device& _device, VkRenderPass renderPass, std::vector<Image>&  attachments
                );
                Framebuffer(
                    Device& _device, VkRenderPass renderPass, std::vector<Image>&& attachments
                );
                Framebuffer(
                    Device& _deivce, VkRenderPass renderPass,
                    const std::vector<VkImageView>& attachments,
                    VkExtent2D extent_
                );
                Framebuffer(Framebuffer& fb);
                Framebuffer(Framebuffer&& fb);
                ~Framebuffer();

                Framebuffer& operator = (Framebuffer& fb);
                Framebuffer& operator = (Framebuffer&& fb);

                VkFramebuffer framebuffer(){ return _framebuffer; }
                VkExtent2D extent(){ return _extent; }
                bool valid(){ return _framebuffer != VK_NULL_HANDLE; }
            private:
                void createFramebuffer(
                    const std::vector<VkImageView>& attachments,
                    VkExtent2D extent_, VkRenderPass renderPass
                );

                Device& device;
                VkFramebuffer _framebuffer;
                VkExtent2D _extent;
        };
    } // namespace gfx
} // namespace shard