#include <shard/ecs/ecs.hpp>

namespace shard{
    ECS::ECS(r2d::Renderer& _renderer):
        renderer{_renderer}
    {
        for(uint32_t i = 0; i < Entity::MAX_ENTITIES; i++){
            availableEntityIDS.push(i);
        }
    }

    Entity ECS::addEntity(){
        assert(activeEntityCount < Entity::MAX_ENTITIES);
        Entity entity = availableEntityIDS.front();
        availableEntityIDS.pop();
        activeEntityCount++;
        return entity;
    }
    void   ECS::removeEntity(const Entity& entity){}
    uint32_t ECS::getSigniture(const Entity& entity) const {
        return Entity::MAX_ENTITIES;
    }
    void ECS::setSigniture(const Entity& entity, uint32_t sig){}
} // namespace shard