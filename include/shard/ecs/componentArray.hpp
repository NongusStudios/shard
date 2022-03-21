#pragma once

#include "entity.hpp"
#include <cassert>
#include <cstddef>
#include <array>

namespace shard{
    template<typename T, size_t size>
    class ComponentArray{
        public:
        
        private:
            std::array<T, size> components         = {};
            std::array<bool, size> componentsAlloc = {false};
    };
} // namespace shard