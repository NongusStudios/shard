#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "../utils.hpp"
#include "device.hpp"

namespace shard{
    namespace gfx{
        class Buffer{
            public:
                Buffer(Device& _device):
                    device{_device}
                {}
                Buffer(
                    Device& _device,
                    size_t sizeb,
                    VkBufferUsageFlags usageFlag,
                    VmaMemoryUsage memUsage,
                    VkMemoryPropertyFlags memProps
                );
                Buffer(
                    Device& _device,
                    size_t sizeb,
                    const void* data,
                    VkBufferUsageFlags usageFlag,
                    VmaMemoryUsage memUsage,
                    VkMemoryPropertyFlags memProps
                );
                Buffer(Buffer& buf);
                Buffer(Buffer&& buf);
                ~Buffer();

                shard_delete_copy_constructors(Buffer);

                Buffer& operator = (Buffer& buf);
                Buffer& operator = (Buffer&& buf);

                VkDescriptorBufferInfo descriptorInfo(
                    VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0
                ){
                    return VkDescriptorBufferInfo{
                        _buffer,
                        offset,
                        size,
                    };
                }
                void* map(){
                    if(_mapped) return _mapped;
                    vmaMapMemory(device.allocator(), _allocation, &_mapped);
                    return _mapped;
                }
                void unmap(){
                    if(!_mapped) return;
                    vmaUnmapMemory(device.allocator(), _allocation);
                    _mapped = nullptr;
                }
                void bindVertex(VkCommandBuffer commandBuffer){
                    VkDeviceSize offset = 0;
                    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &_buffer, &offset);
                }
                void bindIndex(VkCommandBuffer commandBuffer, VkIndexType type){
                    vkCmdBindIndexBuffer(commandBuffer, _buffer, 0, type);
                }
                VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

                bool valid() const {
                    return
                        _allocation != VK_NULL_HANDLE &&
                        _buffer     != VK_NULL_HANDLE
                    ;
                }
                size_t size() const {  return _size; }
                VmaAllocation allocation() { return _allocation; }
                VkBuffer buffer() { return _buffer; }
                const VmaAllocation allocation() const { return _allocation; }
                const VkBuffer buffer() const { return _buffer; }
                VkBufferUsageFlags usage() const { return _usage; }
                void* mappedMemory() { return _mapped; }
                bool mapped() const { return _mapped != nullptr; }
                static VkDeviceSize getAlignment(
                    VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment
                ){
                    if(minOffsetAlignment > 0){
                        return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
                    }
                    return instanceSize;
                }
            private:
                void createBuffer(
                    const void* data, 
                    VmaMemoryUsage memUsage,
                    VkMemoryPropertyFlags memProps
                );

                Device& device;
                VmaAllocation _allocation = VK_NULL_HANDLE;
                size_t _size = 0;
                VkBuffer _buffer = VK_NULL_HANDLE;
                VkBufferUsageFlags _usage;
                void* _mapped = nullptr;
        };
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