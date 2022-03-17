#pragma once

#include <cstdint>

namespace shard{
    class Entity{
        public:
            static constexpr uint32_t MAX_ENTITIES = 5000;

            Entity(): _id{0} {}
            Entity(uint32_t id_): _id{id_} {}

            uint32_t id() const { return _id;      }
            bool valid()  const { return _id != 0; }
        private:
            uint32_t _id;
    };
} // namespace shard
 