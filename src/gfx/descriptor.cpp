#include <shard/gfx/descriptor.hpp>

namespace shard{
    namespace gfx{
        DescriptorSetLayout::DescriptorSetLayout(
            Device& _device,
            const std::map<uint32_t, VkDescriptorSetLayoutBinding>& _bindings
        ):
            device{_device},
            bindings{_bindings}
        {
            std::vector<VkDescriptorSetLayoutBinding> layoutBindings{};
            for(auto binding : bindings){
                layoutBindings.push_back(binding.second);
            }

            VkDescriptorSetLayoutCreateInfo layoutInfo = {};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
            layoutInfo.pBindings = layoutBindings.data();

            shard_abort_ifnot(
                vkCreateDescriptorSetLayout(
                    device.device(),
                    &layoutInfo,
                    nullptr,
                    &_layout
                ) == VK_SUCCESS
            );
        }
        DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout& dsl):
            device{dsl.device},
            _layout{dsl._layout},
            bindings{dsl.bindings}
        {
            //assert(dsl.valid());
            dsl._layout = VK_NULL_HANDLE;
            dsl.bindings = {};
        }
        DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& dsl):
            device{dsl.device},
            _layout{dsl._layout},
            bindings{dsl.bindings}
        {
            //assert(dsl.valid());
            dsl._layout = VK_NULL_HANDLE;
            dsl.bindings = {};
        }
        DescriptorSetLayout::~DescriptorSetLayout(){
            vkDestroyDescriptorSetLayout(device.device(), _layout, nullptr);
        }

        DescriptorSetLayout& DescriptorSetLayout::operator = (DescriptorSetLayout& dsl){
            assert(&device == &dsl.device);
            vkDestroyDescriptorSetLayout(device.device(), _layout, nullptr);
            _layout = dsl._layout;
            bindings = dsl.bindings;

            dsl._layout = VK_NULL_HANDLE;
            dsl.bindings = {};
            return *this;
        }
        DescriptorSetLayout& DescriptorSetLayout::operator = (DescriptorSetLayout&& dsl){
            assert(&device == &dsl.device);
            vkDestroyDescriptorSetLayout(device.device(), _layout, nullptr);
            _layout = dsl._layout;
            bindings = dsl.bindings;

            dsl._layout = VK_NULL_HANDLE;
            dsl.bindings = {};
            return *this;
        }

        DescriptorPool::DescriptorPool(
            Device& _device,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags flags,
            const std::vector<VkDescriptorPoolSize>& sizes
        ):
            device{_device}
        {
            VkDescriptorPoolCreateInfo descriptorPoolInfo{};
            descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(sizes.size());
            descriptorPoolInfo.pPoolSizes = sizes.data();
            descriptorPoolInfo.maxSets = maxSets;
            descriptorPoolInfo.flags = flags;

            shard_abort_ifnot(
                vkCreateDescriptorPool(
                    device.device(),
                    &descriptorPoolInfo,
                    nullptr,
                    &_pool
                ) == VK_SUCCESS
            );

            for(const auto& size : sizes){
                if(!sizeCounts.contains(size.type)) sizeCounts[size.type] = 0;
                sizeCounts[size.type] += size.descriptorCount;
            }
        }
        DescriptorPool::DescriptorPool(DescriptorPool& dp):
            device{dp.device},
            _pool{dp._pool}
        {
            //assert(dp.valid());
            dp._pool = VK_NULL_HANDLE;
        }
        DescriptorPool::DescriptorPool(DescriptorPool&& dp):
            device{dp.device},
            _pool{dp._pool}
        {
            //assert(dp.valid());
            dp._pool = VK_NULL_HANDLE;
        }
        DescriptorPool::~DescriptorPool(){
            vkDestroyDescriptorPool(device.device(), _pool, nullptr);
        }

        DescriptorPool& DescriptorPool::operator = (DescriptorPool& dp){
            assert(&device == &dp.device);
            vkDestroyDescriptorPool(device.device(), _pool, nullptr);
            _pool = dp._pool;
            dp._pool = VK_NULL_HANDLE;
            return *this;
        }
        DescriptorPool& DescriptorPool::operator = (DescriptorPool&& dp){
            assert(&device == &dp.device);
            vkDestroyDescriptorPool(device.device(), _pool, nullptr);
            _pool = dp._pool;
            dp._pool = VK_NULL_HANDLE;
            return *this;
        }

        void DescriptorPool::allocateDescriptor(
            VkDescriptorSetLayout layout,
            VkDescriptorSet& descriptor
        ){
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = _pool;
            allocInfo.pSetLayouts = &layout;
            allocInfo.descriptorSetCount = 1;
            
            shard_abort_ifnot(
                vkAllocateDescriptorSets(
                    device.device(),
                    &allocInfo,
                    &descriptor
                ) == VK_SUCCESS
            );
        }
        void DescriptorPool::freeDescriptor(
            VkDescriptorSet& descriptor
        ){
            vkFreeDescriptorSets(
                device.device(),
                _pool, 1,
                &descriptor
            );
        }
        void DescriptorPool::freeDescriptors(
            std::vector<VkDescriptorSet>& descriptors
        ){
            vkFreeDescriptorSets(
                device.device(),
                _pool, descriptors.size(),
                descriptors.data()
            );
        }

        void DescriptorPool::reset(){
            vkResetDescriptorPool(
                device.device(),
                _pool,
                0
            );
        }

        DescriptorWriter& DescriptorWriter::writeBuffer(
            uint32_t binding, VkDescriptorBufferInfo* bufferInfo
        ){
            assert(setLayout.bindings.contains(binding));

            auto& desc = setLayout.bindings[binding];

            assert(
                desc.descriptorCount == 1 &&
                "Binding single descriptor info, but binding expects multiple"
            );

            VkWriteDescriptorSet write = {};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.descriptorType = desc.descriptorType;
            write.dstBinding = binding;
            write.pBufferInfo = bufferInfo;
            write.descriptorCount = 1;

             writes.push_back(write);
             return *this;
        }
        DescriptorWriter& DescriptorWriter::writeImage(
            uint32_t binding, VkDescriptorImageInfo* imageInfo
        ){
            assert(setLayout.bindings.contains(binding));

            auto& desc = setLayout.bindings[binding];

            assert(
                desc.descriptorCount == 1 &&
                "Binding single descriptor info, but binding expects multiple"
            );

            VkWriteDescriptorSet write = {};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.descriptorType = desc.descriptorType;
            write.dstBinding = binding;
            write.pImageInfo = imageInfo;
            write.descriptorCount = 1;

            writes.push_back(write);
            return *this;
        }

        void DescriptorWriter::build(VkDescriptorSet& set){
            pool.allocateDescriptor(setLayout.layout(), set);
            overwrite(set);
        }
        void DescriptorWriter::overwrite(VkDescriptorSet& set){
            for (auto &write : writes) {
                write.dstSet = set;
            }
            vkUpdateDescriptorSets(
                pool.device.device(), writes.size(), writes.data(), 0, nullptr
            );
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