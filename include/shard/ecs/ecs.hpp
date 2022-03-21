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
            virtual void create(ECS* ecs, const Entity& entity, const Component* data){}
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

            ComponentArray<T>& add(const Entity& entity){
                assert(entity.valid());
                assert(!contains(entity));

                size_t idx = availableComponents.front();
                availableComponents.pop();

                entityToIndex[entity.id()] = idx;
                indexToEntity[idx] = entity.id();
                
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
            ComponentArray<T>& remove(const Entity& entity){
                assert(entity.valid());
                assert(contains(entity));

                size_t idx = entityToIndex[entity.id()];

                components[idx] = {};

                entityToIndex.erase(entity.id());
                indexToEntity.erase(idx);

                availableComponents.push(idx);

                return *this;
            }

            size_t size() const {
                return components.size();
            }
            bool contains(const Entity& entity) const {
                return entityToIndex.contains(entity.id());
            }

            std::vector<std::pair<Entity, T&>> getComponents(){
                std::vector<T&> vec = {};
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
                uint32_t id =  availableEntities.front();
                availableEntities.pop();
                return id;
            }
            ECS& removeEntity(const Entity& entity){
                assert(entity.valid());
                assert(allocatedEntities.contains(entity.id()));
                availableEntities.push(entity.id());
                return  *this;
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

            void update(){

            }
        private:
            std::queue<uint32_t> availableEntities;
            std::unordered_map<uint32_t, uint32_t> allocatedEntities;

            std::unordered_map<std::string, void*> resources;
            std::unordered_map<std::string, const void*> constResources;
            std::unordered_map<std::string, void*> componentArrays;
            std::unordered_map<std::string, std::pair<System, std::string>> systems;
    };
} // namespace shard
