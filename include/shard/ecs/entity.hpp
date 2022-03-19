#pragma once

#include <cstdint>

namespace shard{
    class Entity{
        public:
            Entity(): _id{0} {}
            Entity(uint32_t id_): _id{id_} {}

            uint32_t id() const { return _id;      }
        private:
            uint32_t _id;
    };
} // namespace shard
 