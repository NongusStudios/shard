#pragma once

#include <queue>
#include <array>

#include "../renderer/renderer2d.hpp"
#include "system.hpp"

namespace shard{
    class ECS{
        public:
            ECS(r2d::Renderer& _renderer);

            Entity addEntity();
            void   removeEntity(const Entity& entity);
            uint32_t getSigniture(const Entity& entity) const;
            void setSigniture(const Entity& entity, uint32_t sig);
        private:
            r2d::Renderer& renderer;

            std::queue<uint32_t> availableEntityIDS;
            std::array<uint32_t, Entity::MAX_ENTITIES> signitures;
            uint32_t activeEntityCount = 0;
    };
} // namespace shard
