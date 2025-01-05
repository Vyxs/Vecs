//
// Created by Vyxs on 05/01/2025.
//

#ifndef ECS_H
#define ECS_H

#include "Entity.h"
#include "Pool.h"
#include <memory>
#include <unordered_map>
#include <typeindex>

namespace vecs {
    /**
     * @brief Core ECS (Entity Component System) implementation
     */
    class ECS {
        EntityManager entityManager;
        std::unordered_map<std::type_index, std::unique_ptr<BasePool>> pools;

        template<typename T>
        [[nodiscard]] Pool<T>& getPool() {
            const auto typeIndex = std::type_index(typeid(T));
            const auto it = pools.find(typeIndex);
            if (it == pools.end()) {
                auto [inserted, success] = pools.try_emplace(
                    typeIndex,
                    std::make_unique<Pool<T>>()
                );
                return *static_cast<Pool<T>*>(inserted->second.get());
            }
            return *static_cast<Pool<T>*>(it->second.get());
        }

        template<typename T>
        [[nodiscard]] const Pool<T>* tryGetPool() const noexcept {
            const auto typeIndex = std::type_index(typeid(T));
            const auto it = pools.find(typeIndex);
            if (it == pools.end()) {
                return nullptr;
            }
            return static_cast<const Pool<T>*>(it->second.get());
        }

    public:
        explicit ECS(const size_t initialEntityCapacity = 1024)
            : entityManager(initialEntityCapacity) {}

        /**
         * @brief Creates a new entity
         * @return Newly created entity
         */
        [[nodiscard]] Entity createEntity() noexcept {
            return entityManager.create();
        }

        /**
         * @brief Checks if an entity is valid
         * @param entity Entity to check
         * @return True if entity is valid
         */
        [[nodiscard]] bool isValid(const Entity entity) const noexcept {
            return entityManager.isValid(entity);
        }

        /**
         * @brief Destroys an entity and all its components
         * @param entity Entity to destroy
         */
        void destroyEntity(const Entity entity) noexcept {
            if (!isValid(entity)) return;

            for (auto& [_, pool] : pools) {
                pool->removeEntity(entity);
            }

            entityManager.destroy(entity);
        }

        /**
         * @brief Adds a component to an entity
         * @param entity Target entity
         * @param component Component to add
         * @return Reference to the added component
         * @throws std::runtime_error if entity is invalid
         */
        template<typename T>
        T& addComponent(Entity entity, T&& component) {
            if (!isValid(entity)) {
                throw std::runtime_error("Invalid entity");
            }

            auto& pool = getPool<T>();
            pool.insert(entity, std::forward<T>(component));
            return pool.get(entity);
        }

        /**
         * @brief Constructs a component in place for an entity
         * @param entity Target entity
         * @param args Arguments for component construction
         * @return Reference to the created component
         * @throws std::runtime_error if entity is invalid
         */
        template<typename T, typename... Args>
        T& emplaceComponent(Entity entity, Args&&... args) {
            if (!isValid(entity)) {
                throw std::runtime_error("Invalid entity");
            }

            return getPool<T>().emplace(entity, std::forward<Args>(args)...);
        }

        /**
         * @brief Replaces or adds a component to an entity
         * @param entity Target entity
         * @param component New component value
         * @return Reference to the component
         * @throws std::runtime_error if entity is invalid
         */
        template<typename T>
        T& replaceComponent(Entity entity, T&& component) {
            if (!isValid(entity)) {
                throw std::runtime_error("Invalid entity");
            }

            auto& pool = getPool<T>();
            if (!pool.has(entity)) {
                pool.insert(entity, std::forward<T>(component));
            } else {
                pool.get(entity) = std::forward<T>(component);
            }
            return pool.get(entity);
        }

        /**
         * @brief Gets a component from an entity
         * @param entity Target entity
         * @return Reference to the component
         * @throws std::runtime_error if entity is invalid or component not found
         */
        template<typename T>
        [[nodiscard]] T& getComponent(Entity entity) {
            if (!isValid(entity)) {
                throw std::runtime_error("Invalid entity");
            }
            return getPool<T>().get(entity);
        }

        /**
         * @brief Gets a component from an entity (const)
         * @param entity Target entity
         * @return Const reference to the component
         * @throws std::runtime_error if entity is invalid or component not found
         */
        template<typename T>
        [[nodiscard]] const T& getComponent(Entity entity) const {
            if (!isValid(entity)) {
                throw std::runtime_error("Invalid entity");
            }

            const auto* pool = tryGetPool<T>();
            if (!pool) {
                throw std::runtime_error("Component type not found");
            }
            return pool->get(entity);
        }

        /**
         * @brief Checks if an entity has a component
         * @param entity Target entity
         * @return True if entity has the component
         */
        template<typename T>
        [[nodiscard]] bool hasComponent(Entity entity) const noexcept {
            if (!isValid(entity)) return false;

            const auto* pool = tryGetPool<T>();
            if (!pool) return false;

            return pool->has(entity);
        }

        /**
         * @brief Checks if an entity has all specified components
         * @param entity Target entity
         * @return True if entity has all components
         */
        template<typename First, typename... Rest>
        [[nodiscard]] bool hasComponents(const Entity entity) const noexcept {
            return hasComponent<First>(entity) && (hasComponent<Rest>(entity) && ...);
        }

        /**
         * @brief Removes a component from an entity
         * @param entity Target entity
         */
        template<typename T>
        void removeComponent(Entity entity) noexcept {
            if (!isValid(entity)) return;

            if (auto* pool = const_cast<Pool<T>*>(tryGetPool<T>())) {
                pool->removeEntity(entity);
            }
        }

        /**
         * @brief Removes multiple components from an entity
         * @param entity Target entity
         */
        template<typename First, typename... Rest>
        void removeComponents(const Entity entity) noexcept {
            removeComponent<First>(entity);
            (removeComponent<Rest>(entity), ...);
        }

        /**
         * @brief Clears all entities and components
         */
        void clear() noexcept {
            for (auto& [_, pool] : pools) {
                pool->clear();
            }
            entityManager.clear();
        }

        /**
         * @brief Gets the number of active entities
         */
        [[nodiscard]] size_t size() const noexcept {
            return entityManager.size();
        }

        /**
         * @brief Gets the current entity capacity
         */
        [[nodiscard]] size_t getEntityCapacity() const noexcept {
            return entityManager.capacity();
        }

        /**
         * @brief Gets a component pool
         * @return Pointer to the component pool or nullptr if not found
         */
        template<typename T>
        [[nodiscard]] const Pool<T>* getComponentPool() const noexcept {
            return tryGetPool<T>();
        }
    };
}

#endif