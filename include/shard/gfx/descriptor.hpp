#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <map>

#include "device.hpp"
#include "../def.hpp"
#include "../utils.hpp"

namespace shard{
    namespace gfx{
        class DescriptorSetLayout{
            public:
                class Builder{
                    public:
                        Builder(Device& _device):
                            device{_device}
                        {}

                        Builder& addBinding(
                            uint32_t binding,
                            VkDescriptorType type,
                            VkShaderStageFlags stageFlags,
                            uint32_t count = 1
                        ){
                            assert(!bindings.contains(binding));
                            VkDescriptorSetLayoutBinding layout = {};
                            layout.binding = binding;
                            layout.descriptorType = type;
                            layout.descriptorCount = count;
                            layout.stageFlags = stageFlags;
                            bindings[binding] = layout;
                            return *this;
                        }

                        DescriptorSetLayout build(){
                            return DescriptorSetLayout(device, bindings);
                        }
                        
                    private:
                        Device& device;
                        std::map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
                };

                DescriptorSetLayout(
                    Device& _device,
                    const std::map<uint32_t, VkDescriptorSetLayoutBinding>& _bindings
                );
                DescriptorSetLayout(DescriptorSetLayout& dsl);
                DescriptorSetLayout(DescriptorSetLayout&& dsl);
                ~DescriptorSetLayout();

                DescriptorSetLayout& operator = (DescriptorSetLayout& dsl);
                DescriptorSetLayout& operator = (DescriptorSetLayout&& dsl);

                VkDescriptorSetLayout layout() { return _layout; }
                bool valid() const { return _layout != VK_NULL_HANDLE; }
            private:
                Device& device;
                VkDescriptorSetLayout _layout;
                std::map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

                friend class DescriptorWriter;
        };

        class DescriptorPool{
            public:
                class Builder{
                    public:
                        Builder(Device& _device):
                            device{_device}
                        {}

                        Builder& addPoolSize(VkDescriptorType type, uint32_t count){
                            poolSizes.push_back({type, count});
                            return *this;
                        }
                        Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags){
                            poolFlags = flags;
                            return *this;
                        }
                        Builder& setMaxSets(uint32_t count){
                            maxSets = count;
                            return *this;
                        }
                        DescriptorPool build(){
                            return DescriptorPool(
                                device,
                                maxSets,
                                poolFlags,
                                poolSizes
                            );
                        }

                    private:
                        Device& device;
                        std::vector<VkDescriptorPoolSize> poolSizes{};
                        uint32_t maxSets = 1000;
                        VkDescriptorPoolCreateFlags poolFlags = 0;
                };

                DescriptorPool(Device& _device):
                    device{_device},
                    _pool{VK_NULL_HANDLE}
                {}
                DescriptorPool(
                    Device& _device,
                    uint32_t maxSets,
                    VkDescriptorPoolCreateFlags flags,
                    const std::vector<VkDescriptorPoolSize>& sizes
                );
                DescriptorPool(DescriptorPool& dp);
                DescriptorPool(DescriptorPool&& dp);
                ~DescriptorPool();

                DescriptorPool& operator = (DescriptorPool& dp);
                DescriptorPool& operator = (DescriptorPool&& dp);

                VkDescriptorPool pool() { return _pool; }
                uint32_t getSizeCount(VkDescriptorType type){
                    assert(sizeCounts.contains(type));
                    return sizeCounts[type];
                }
                bool valid() const { return _pool != VK_NULL_HANDLE; }

                void allocateDescriptor(
                    VkDescriptorSetLayout layout,
                    VkDescriptorSet& descriptor
                );
                void freeDescriptor(
                    VkDescriptorSet& descriptor
                );
                void freeDescriptors(
                    std::vector<VkDescriptorSet>& descriptors
                );

                void reset();
            private:
                Device& device;
                VkDescriptorPool _pool;
                std::map<VkDescriptorType, uint32_t> sizeCounts;

                friend class DescriptorWriter;
        };

        class DescriptorWriter{
            public:
                DescriptorWriter(DescriptorSetLayout& _setLayout, DescriptorPool& _pool):
                    setLayout{_setLayout},
                    pool{_pool}
                {}
                
                DescriptorWriter& writeBuffer(
                    uint32_t binding, VkDescriptorBufferInfo* bufferInfo
                );
                DescriptorWriter& writeImage(
                    uint32_t binding, VkDescriptorImageInfo* imageInfo
                );

                void build(VkDescriptorSet& set);
                void overwrite(VkDescriptorSet& set);
            private:
                DescriptorSetLayout& setLayout;
                DescriptorPool& pool;
                std::vector<VkWriteDescriptorSet> writes;
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