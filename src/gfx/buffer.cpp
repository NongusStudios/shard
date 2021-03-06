#include <shard/gfx/buffer.hpp>
#include <memory.h>

namespace shard{
    namespace gfx{
        Buffer::Buffer(
            Device& _device,
            size_t sizeb,
            VkBufferUsageFlags usageFlag,
            VmaMemoryUsage memUsage,
            VkMemoryPropertyFlags memProps,
            VkSharingMode sharingMode
        ):
            device{_device},
            _size{sizeb}
        {
            createBuffer(nullptr, usageFlag, memUsage, memProps, sharingMode);
        }
        Buffer::Buffer(
            Device& _device,
            size_t sizeb,
            const void* data,
            VkBufferUsageFlags usageFlag,
            VmaMemoryUsage memUsage,
            VkMemoryPropertyFlags memProps,
            VkSharingMode sharingMode
        ):
            device{_device},
            _size{sizeb}
        {
            createBuffer(data, usageFlag, memUsage, memProps, sharingMode);
        }
        Buffer::Buffer(Buffer& buf):
            device{buf.device}
        {
            //assert(buf.valid());
            _allocation = buf._allocation;
            _buffer = buf._buffer;
            _size = buf._size;
            _mapped = buf._mapped;
            buf._allocation = VK_NULL_HANDLE;
            buf._buffer = VK_NULL_HANDLE;
            buf._mapped = nullptr;
        }
        Buffer::Buffer(Buffer&& buf):
            device{buf.device}
        {
            //assert(buf.valid());
            _allocation = buf._allocation;
            _buffer = buf._buffer;
            _size = buf._size;
            _mapped = buf._mapped;
            buf._allocation = VK_NULL_HANDLE;
            buf._buffer = VK_NULL_HANDLE;
            buf._mapped = nullptr;
        }
        Buffer::~Buffer(){
            unmap();
            vmaDestroyBuffer(device.allocator(), _buffer, _allocation);
        }

        Buffer& Buffer::operator = (Buffer& buf){
            assert(&device == &buf.device);
            device.waitIdle();
            unmap();
            vmaDestroyBuffer(device.allocator(), _buffer, _allocation);
            _allocation = buf._allocation;
            _buffer = buf._buffer;
            _size = buf._size;
            _mapped = buf._mapped;

            buf._allocation = VK_NULL_HANDLE;
            buf._buffer = VK_NULL_HANDLE;
            buf._mapped = nullptr;
            return *this;
        }
        Buffer& Buffer::operator = (Buffer&& buf){
            assert(&device == &buf.device);
            device.waitIdle();
            unmap();
            vmaDestroyBuffer(device.allocator(), _buffer, _allocation);
            _allocation = buf._allocation;
            _buffer = buf._buffer;
            _size = buf._size;
            _mapped = buf._mapped;

            buf._allocation = VK_NULL_HANDLE;
            buf._buffer = VK_NULL_HANDLE;
            buf._mapped = nullptr;
            return *this;
        }

        VkResult Buffer::flush(VkDeviceSize size, VkDeviceSize offset){
            return vmaFlushAllocation(device.allocator(), _allocation, offset, size);
        }

        void Buffer::createBuffer(
            const void* data,
            VkBufferUsageFlags usage,
            VmaMemoryUsage memUsage,
            VkMemoryPropertyFlags memProps,
            VkSharingMode sharingMode
        ){
            if(_size == 0) return;

            VkBufferCreateInfo bufferInfo = {};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = _size;
            bufferInfo.usage = usage;
            bufferInfo.sharingMode = sharingMode;

            VmaAllocationCreateInfo allocInfo = {};
            allocInfo.usage = memUsage;
            allocInfo.requiredFlags = memProps;

            shard_abort_ifnot(
                vmaCreateBuffer(
                    device.allocator(),
                    &bufferInfo, &allocInfo,
                    &_buffer, &_allocation,
                    nullptr
                ) == VK_SUCCESS
            );
            if(memProps & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT){
                map();
                if(data){
                    memcpy(_mapped, data, _size);
                }
            }
        }
    }
}

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