#pragma once

#include "entity.hpp"

namespace shard{
    class ECS;
    template<typename T>
    class System{
        public:
            virtual void update(ECS& ecs, const Entity& entity, T& component){}
    };
} // namespace shard
