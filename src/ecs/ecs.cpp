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
    void   ECS::removeEntity(const Entity& entity){
        assert(entity.valid());
        setSigniture(entity, 0);
        availableEntityIDS.push(entity.id());
        activeEntityCount--;
        activeEntityCount = std::clamp(activeEntityCount, 0u, Entity::MAX_ENTITIES);
    }
    uint32_t ECS::getSigniture(const Entity& entity) const {
        assert(entity.valid());
        return signitures[entity.id()];
    }
    void ECS::setSigniture(const Entity& entity, uint32_t sig){
        assert(entity.valid());
        signitures[entity.id()] = sig;
    }
} // namespace shard