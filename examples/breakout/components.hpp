#pragma once

#include <shard/renderer/renderer2d.hpp>
#include <shard/sound/sound.hpp>
#include <shard/time/time.hpp>
#include <shard/input/input.hpp>
#include <shard/ecs/ecs.hpp>

class Transform : public shard::Component {
    public:
        void create(
            shard::ECS& ecs, const shard::Entity& entity, const shard::Component* component
        ){
            const Transform& transformData = *reinterpret_cast<const Transform*>(component);
            position   = transformData.position;
            rotation   = transformData.rotation;
            angularVel = transformData.angularVel;
            scale      = transformData.scale;
            vel        = transformData.vel;
            accel      = transformData.accel;
        }

        glm::vec2 position;
        float     rotation;
        float     angularVel;
        glm::vec2 scale;
        glm::vec2 vel;
        glm::vec2 accel;
};

class Rect : public shard::Component {
    void create(
            shard::ECS& ecs, const shard::Entity& entity, const shard::Component* component
    ){
        const Rect& rectData = *reinterpret_cast<const Rect*>(component);
        const Transform& transform = ecs.getComponentArray<Transform>("transforms").get(entity);
        shard::r2d::Renderer& r2d = ecs.getResource<shard::r2d::Renderer>("r2d");
    }

    uint32_t rect = 0;
};