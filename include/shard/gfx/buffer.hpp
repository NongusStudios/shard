#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../utils.hpp"
#include "device.hpp"

namespace shard{
    namespace gfx{
        class Buffer{
            public:
                Buffer(
                    Device& _device,
                    size_t sizeb,
                    VkBufferUsageFlagBits usageFlag,
                    VmaMemoryUsage memUsage
                );
                Buffer(
                    Device& _device,
                    size_t sizeb,
                    void* data,
                    VkBufferUsageFlagBits usageFlag,
                    VmaMemoryUsage memUsage
                );
                Buffer(Buffer& buf);
                Buffer(Buffer&& buf);
                ~Buffer();

                Buffer& operator = (Buffer& buf);
                Buffer& operator = (Buffer&& buf);

                void map(void** data){
                    vmaMapMemory(device.allocator(), _allocation, data);
                }
                void unmap(){
                    vmaUnmapMemory(device.allocator(), _allocation);
                }
                void bindVertex(VkCommandBuffer commandBuffer){
                    VkDeviceSize offset = 0;
                    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_buffer, &offset);
                }
                void bindIndex(VkCommandBuffer commandBuffer, VkIndexType type){
                    VkDeviceSize offset = 0;
                    vkCmdBindIndexBuffer(commandBuffer, _buffer, offset, type);
                }

                bool valid(){
                    return
                        _allocation != VK_NULL_HANDLE &&
                        _buffer     != VK_NULL_HANDLE
                    ;
                }
                size_t size(){  return _size; }
                VmaAllocation allocation() { return _allocation; }
                VkBuffer buffer() { return _buffer; }
                VkBufferUsageFlagBits usage() { return _usage; }
            private:
                void createBuffer(void* data, VmaMemoryUsage memUsage);

                Device& device;
                VmaAllocation _allocation = VK_NULL_HANDLE;
                size_t _size;
                VkBuffer _buffer = VK_NULL_HANDLE;
                VkBufferUsageFlagBits _usage;
        };
    } // namespace gfx
    
} // namespace shard
