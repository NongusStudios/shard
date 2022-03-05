#include <shard/gfx/framebuffer.hpp>

#include <cassert>

namespace shard{
    namespace gfx{
        Framebuffer::Framebuffer(
            Device& _device, VkRenderPass renderPass, std::vector<Image>& attachments
        ):
            device{_device}
        {
            assert(renderPass != VK_NULL_HANDLE);
            assert(attachments.size() > 0);
            std::vector<VkImageView> imageViews = {};
            for(auto& image : attachments){
                for(size_t i = 0; i < attachments.size(); i++){
                    assert(image.extent().width  == attachments[i].extent().width );
                    assert(image.extent().height == attachments[i].extent().height);
                }
                imageViews.push_back(image.imageView());
            }
            createFramebuffer(imageViews, attachments[0].extent(), renderPass);
        }
        Framebuffer::Framebuffer(
            Device& _device, VkRenderPass renderPass, std::vector<Image>&& attachments
        ):
            device{_device}
        {
            assert(renderPass != VK_NULL_HANDLE);
            assert(attachments.size() > 0);
            std::vector<VkImageView> imageViews = {};
            for(auto& image : attachments){
                for(size_t i = 0; i < attachments.size(); i++){
                    assert(image.extent().width  == attachments[i].extent().width );
                    assert(image.extent().height == attachments[i].extent().height);
                }
                imageViews.push_back(image.imageView());
            }
            createFramebuffer(imageViews, attachments[0].extent(), renderPass);
        }
        Framebuffer::Framebuffer(
            Device& _deivce, VkRenderPass renderPass,
            const std::vector<VkImageView>& attachments,
            VkExtent2D extent_
        ):
            device{_deivce}
        {
            assert(renderPass != VK_NULL_HANDLE);
            assert(attachments.size() > 0);
            assert(extent_.width*extent_.height > 0);
            createFramebuffer(attachments, extent_, renderPass);
        }
        Framebuffer::Framebuffer(Framebuffer& fb):
            device{fb.device},
            _framebuffer{fb._framebuffer},
            _extent{fb._extent}
        {
            //assert(fb.valid());
            fb._framebuffer = VK_NULL_HANDLE;
        }
        Framebuffer::Framebuffer(Framebuffer&& fb):
            device{fb.device},
            _framebuffer{fb._framebuffer},
            _extent{fb._extent}
        {
            //assert(fb.valid());
            fb._framebuffer = VK_NULL_HANDLE;
        }
        Framebuffer::~Framebuffer(){
            vkDestroyFramebuffer(device.device(), _framebuffer, nullptr);
        }

        Framebuffer& Framebuffer::operator = (Framebuffer& fb){
            assert(&device == &fb.device);
            vkDestroyFramebuffer(device.device(), _framebuffer, nullptr);
            _framebuffer = fb._framebuffer;
            _extent = fb._extent;
            fb._framebuffer = VK_NULL_HANDLE;
            return *this;
        }
        Framebuffer& Framebuffer::operator = (Framebuffer&& fb){
            assert(&device == &fb.device);
            vkDestroyFramebuffer(device.device(), _framebuffer, nullptr);
            _framebuffer = fb._framebuffer;
            _extent = fb._extent;
            fb._framebuffer = VK_NULL_HANDLE;
            return *this;
        }

        void Framebuffer::createFramebuffer(
            const std::vector<VkImageView>& attachments,
            VkExtent2D extent_, VkRenderPass renderPass
        ){
            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = extent_.width;
            framebufferInfo.height = extent_.height;
            framebufferInfo.layers = 1;

            shard_abort_ifnot(
                vkCreateFramebuffer(
                    device.device(), &framebufferInfo, nullptr, &_framebuffer
                ) == VK_SUCCESS
            );

            _extent = extent_;
        }
    } // namespace gfx
} // namespace shard
