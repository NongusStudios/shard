#include <shard/gfx/buffer.hpp>
#include <memory.h>

namespace shard{
    namespace gfx{
        Buffer::Buffer(
            Device& _device,
            size_t sizeb,
            VkBufferUsageFlagBits usageFlag,
            VmaMemoryUsage memUsage
        ):
            device{_device},
            _size{sizeb},
            _usage{usageFlag}
        {
            createBuffer(nullptr, memUsage);
        }
        Buffer::Buffer(
            Device& _device,
            size_t sizeb,
            void* data,
            VkBufferUsageFlagBits usageFlag,
            VmaMemoryUsage memUsage
        ):
            device{_device},
            _size{sizeb},
            _usage{usageFlag}
        {
            createBuffer(data, memUsage);
        }
        Buffer::Buffer(Buffer& buf):
            device{buf.device}
        {
            assert(buf.valid());
            _allocation = buf._allocation;
            _buffer = buf._buffer;
            _usage = buf._usage;
            buf._allocation = VK_NULL_HANDLE;
            buf._buffer = VK_NULL_HANDLE;
        }
        Buffer::Buffer(Buffer&& buf):
            device{buf.device}
        {
            assert(buf.valid());
            _allocation = buf._allocation;
            _buffer = buf._buffer;
            _usage = buf._usage;
            buf._allocation = VK_NULL_HANDLE;
            buf._buffer = VK_NULL_HANDLE;
        }
        Buffer::~Buffer(){
            vmaDestroyBuffer(device.allocator(), _buffer, _allocation);
        }

        Buffer& Buffer::operator = (Buffer& buf){
            assert(&device == &buf.device);
            device.waitIdle();
            vmaDestroyBuffer(device.allocator(), _buffer, _allocation);
            _allocation = buf._allocation;
            _buffer = buf._buffer;
            _usage = buf._usage;

            buf._allocation = VK_NULL_HANDLE;
            buf._buffer = VK_NULL_HANDLE;
        }
        Buffer& Buffer::operator = (Buffer&& buf){
            assert(&device == &buf.device);
            device.waitIdle();
            vmaDestroyBuffer(device.allocator(), _buffer, _allocation);
            _allocation = buf._allocation;
            _buffer = buf._buffer;
            _usage = buf._usage;

            buf._allocation = VK_NULL_HANDLE;
            buf._buffer = VK_NULL_HANDLE;
        }

        void Buffer::createBuffer(void* data, VmaMemoryUsage memUsage){
            if(_size == 0) return;

            VkBufferCreateInfo bufferInfo = {};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = _size;
            bufferInfo.usage = _usage;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo allocInfo = {};
            allocInfo.usage = memUsage;

            shard_abort_ifnot(
                vmaCreateBuffer(
                    device.allocator(),
                    &bufferInfo, &allocInfo,
                    &_buffer, &_allocation,
                    nullptr
                ) == VK_SUCCESS
            );

            if(data){
                void* _data;
                map(&_data);
                memcpy(_data, data, _size);
                //unmap();
            }
        }
    }
}