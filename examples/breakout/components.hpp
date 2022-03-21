#pragma once

#include <shard/renderer/renderer2d.hpp>
#include <shard/sound/sound.hpp>
#include <shard/time/time.hpp>
#include <shard/input/input.hpp>
#include <shard/ecs/ecs.hpp>

#include <iostream>

struct Transform : public shard::Component {
    public:
        void create(shard::ECS& ecs, const shard::Entity& entity){}
        void destroy(shard::ECS& ecs, const shard::Entity& entity){}

        glm::vec2 position = {};
        float     rotation = 0.0f;
        float     angularVel = 0.0f;
        glm::vec2 scale = {};
        glm::vec2 vel = {};
        glm::vec2 accel = {};
};

struct Rect : public shard::Component {
    void create(shard::ECS& ecs, const shard::Entity& entity){
        const Transform& transform = ecs.getComponentArray<Transform>("transforms").get(entity);
        shard::r2d::Renderer& r2d = ecs.getResource<shard::r2d::Renderer>("r2d");
        rect = r2d.addRect(
            transform.position, transform.rotation,
            transform.scale, {255.0f}, 0.0f
        );
    }
    void destroy(shard::ECS& ecs, const shard::Entity& entity){
        shard::r2d::Renderer& r2d = ecs.getResource<shard::r2d::Renderer>("r2d");
        r2d.removeRect(rect);
    }

    uint32_t rect = 0;
};