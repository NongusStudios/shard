#pragma once

#include <cstdint>
#include <cassert>
#include <vector>
#include <queue>
#include <string>
#include <unordered_map>
#include <functional>

namespace shard{
    class ECS;

    class Entity{
        public:
            static constexpr uint32_t MAX_ENTITIES = 10000;

            Entity(){}
            Entity(uint32_t id_): _id{id_} {}

            uint32_t id() const { return _id; }
            bool  valid() const { return _id < MAX_ENTITIES; }
        private:
            uint32_t _id = MAX_ENTITIES;
    };

    class Component{
        public:
            virtual void create(ECS& ecs, const Entity& entity, const Component* init){}
            virtual void destroy(ECS& ecs, const Entity& entity){}
    };

    template<typename T>
    class ComponentArray{
        public:
            static constexpr uint32_t DEFAULT_COMPONENT_COUNT = 1000;

            ComponentArray():
                ComponentArray(DEFAULT_COMPONENT_COUNT)
            {}
            ComponentArray(size_t size){
                components.resize(size, {});

                for(size_t i = 0; i < size; i++){
                    availableComponents.push(i);
                }
            }

            ComponentArray<T>& add(ECS& ecs, const Entity& entity, const T& data){
                assert(entity.valid());
                assert(!contains(entity));
                assert(availableComponents.size() > 0);

                size_t idx = availableComponents.front();
                availableComponents.pop();

                entityToIndex[entity.id()] = idx;
                indexToEntity[idx] = entity.id();

                const Component* c = reinterpret_cast<const Component*>(&data);
                components[idx].create(ecs, entity, c);
                
                return *this;
            }
            ComponentArray<T>& set(const Entity& entity, const T& data){
                assert(entity.valid());
                assert(contains(entity));

                size_t idx = entityToIndex[entity.id()];
                components[idx] = data;

                return *this;
            }
            T& get(const Entity& entity){
                assert(entity.valid());
                assert(contains(entity));

                size_t idx = entityToIndex[entity.id()];
                return components[idx];
            }
            ComponentArray<T>& remove(ECS& ecs, const Entity& entity){
                assert(entity.valid());
                assert(contains(entity));

                size_t idx = entityToIndex[entity.id()];

                components[idx].destroy(ecs, entity);
                components[idx] = {};

                entityToIndex.erase(entity.id());
                indexToEntity.erase(idx);

                availableComponents.push(idx);

                return *this;
            }

            Entity getEntityFromIndex(size_t idx){
                assert(indexToEntity.contains(idx));
                return Entity(indexToEntity[idx]);
            }
            size_t getIndexFromEntity(const Entity& entity){
                assert(entityToIndex.contains(entity.id()));
                return entityToIndex[entity.id()];
            }

            size_t size() const {
                return components.size();
            }
            bool contains(const Entity& entity) const {
                return entityToIndex.contains(entity.id());
            }

            std::vector<std::pair<Entity, T&>> getComponents(){
                std::vector<std::pair<Entity, T&>> vec = {};
                for(auto& [entity, index] : entityToIndex){
                    vec.push_back({Entity{entity}, components[index]});
                }
                return vec;
            }
        private:
            std::queue<size_t>   availableComponents;
            std::vector<T>       components;

            std::unordered_map<uint32_t, size_t> entityToIndex;
            std::unordered_map<size_t, uint32_t> indexToEntity;
    };
    
    using System = std::function<void(ECS&, const Entity&, Component*)>;

    class ECS{
        public:
            ECS(){
                for(uint32_t i = 0; i < Entity::MAX_ENTITIES; i++){
                    availableEntities.push(i);
                }
            }

            Entity addEntity(){
                assert(availableEntities.size() > 0);
                uint32_t id = availableEntities.front();
                availableEntities.pop();
                allocatedEntities[id] = id;
                return id;
            }
            ECS& removeEntity(const Entity& entity){
                assert(entity.valid());
                assert(allocatedEntities.contains(entity.id()));
                assert(!entityConnections.contains(entity.id()));
                availableEntities.push(entity.id());
                allocatedEntities.erase(entity.id());
                return  *this;
            }

            template<typename T>
            ECS& insertComponent(const Entity& entity, const std::string& componentArray, const T& data){
                assert(allocatedEntities.contains(entity.id()));
                ComponentArray<T>& arr = getComponentArray<T>(componentArray);
                arr.add(*this, entity, data);
                entityConnections[entity.id()][componentArray];
                return *this;
            }
            template<typename T>
            ECS& removeComponent(const Entity& entity, const std::string& componentArray){
                assert(allocatedEntities.contains(entity.id()));
                assert(entityConnections.contains(entity.id()));
                assert(entityConnections[entity.id()].contains(componentArray));
                ComponentArray<T>& arr = getComponentArray<T>(componentArray);
                arr.remove(*this, entity);
                entityConnections[entity.id()].erase(componentArray);
                if(entityConnections[entity.id()].size() == 0) entityConnections.erase(entity.id());
                return *this;
            }

            template<typename T>
            ECS& insertResource(const std::string& name, T& res){
                assert(!resources.contains(name));
                resources[name] = reinterpret_cast<void*>(&res);
                return *this;
            }
            template<typename T>
            ECS& insertConstantResource(const std::string& name, const T& res){
                assert(!constResources.contains(name));
                constResources[name] = reinterpret_cast<const void*>(&res);
                return *this;
            }
            ECS& insertSystem(
                const std::string& name, const std::string& componentArray, System system
            ){
                assert(!systems.contains(name));
                assert(componentArrays.contains(componentArray));
                systems[name] = {system, componentArray};
                return *this;
            }
            template<typename T>
            ECS& insertComponentArray(const std::string& name, ComponentArray<T>& arr){
                assert(!componentArrays.contains(name));
                componentArrays[name] = reinterpret_cast<void*>(&arr);
                return* this;
            }

            template<typename T>
            T& getResource(const std::string& name){
                assert(resources.contains(name));
                return *reinterpret_cast<T*>(resources[name]);
            }
            bool hasResource(const std::string& name) const {
                return resources.contains(name);
            }
            template<typename T>
            const T& getConstantResource(const std::string& name){
                assert(constResources.contains(name));
                return *reinterpret_cast<const T*>(constResources[name]);
            }
            template<typename T>
            ComponentArray<T>& getComponentArray(const std::string& name){
                assert(componentArrays.contains(name));
                return *reinterpret_cast<ComponentArray<T>*>(componentArrays[name]);
            }
            bool hasComponentArray(const std::string& name) const {
                return componentArrays.contains(name);
            }

            template<typename T>
            ECS& update(const std::string& systemName){
                assert(systems.contains(systemName));
                auto& [system, componentArrayName] = systems[systemName];
                std::vector<std::pair<Entity, T&>> components = getComponentArray<T>(componentArrayName)
                                                                .getComponents();
                for(auto [entity, component] : components){
                    system(*this, entity, reinterpret_cast<Component*>(&component));
                }
                return *this;
            }
        private:
            std::queue<uint32_t> availableEntities;
            std::unordered_map<uint32_t, uint32_t> allocatedEntities;
            std::unordered_map<uint32_t, std::unordered_map<std::string, uint8_t>> entityConnections;

            std::unordered_map<std::string, void*>       resources;
            std::unordered_map<std::string, const void*> constResources;
            std::unordered_map<std::string, void*>       componentArrays;
            std::unordered_map<std::string, std::pair<System, std::string>> systems;
    };
} // namespace shard