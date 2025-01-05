//
// Created by Vyxs on 05/01/2025.
//

#ifndef POOL_H
#define POOL_H

#include "SparseSet.h"

namespace vecs {

    class BasePool {
    public:
        virtual ~BasePool() = default;
        virtual void removeEntity(Entity entity) = 0;
        [[nodiscard]] virtual size_t size() const = 0;
        virtual void clear() = 0;
        virtual void reserve(size_t capacity) = 0;
    };

    /**
     * @brief Pool for storing components of a specific type
     * @tparam T Component type
     * @tparam Allocator Memory allocator type
     */
    template<typename T, typename Allocator = DefaultAllocator<T>>
    class Pool final : public BasePool {

        using SparseSetType = SparseSet<T, Allocator>;
        SparseSetType components;

        size_t componentCount = 0;
        static constexpr size_t cacheLineSize = 64;
        static constexpr size_t minCapacity = 64;

        alignas(cacheLineSize) Entity lastEntity = Entity::null();
        alignas(cacheLineSize) T* lastComponent = nullptr;

    public:
        /**
         * @brief Constructs a Pool with initial capacity
         */
        explicit Pool(size_t initialCapacity = minCapacity)
            : components(initialCapacity) {}

        /**
         * @brief Inserts a component for an entity
         */
        void insert(Entity entity, T&& component) {
            components.insert(entity, std::forward<T>(component));
            componentCount++;

            lastEntity = entity;
            lastComponent = &components.get(entity);
        }

        /**
         * @brief Constructs a component in place for an entity
         */
        template<typename... Args>
        T& emplace(Entity entity, Args&&... args) {
            componentCount += !components.contains(entity);
            T& component = components.emplace(entity, std::forward<Args>(args)...);

            lastEntity = entity;
            lastComponent = &component;
            return component;
        }

        /**
         * @brief Gets the component for an entity
         */
        [[nodiscard]] inline T& get(Entity entity) {
            if (entity == lastEntity) {
                return *lastComponent;
            }

            T& component = components.get(entity);
            lastEntity = entity;
            lastComponent = &component;
            return component;
        }

        /**
         * @brief Gets the component for an entity (const)
         */
        [[nodiscard]] inline const T& get(Entity entity) const {
            return components.get(entity);
        }

        /**
         * @brief Checks if an entity has a component
         */
        [[nodiscard]] inline bool has(Entity entity) const {
            if (entity == lastEntity) {
                return true;
            }
            return components.contains(entity);
        }

        /**
         * @brief Removes a component from an entity
         */
        void removeEntity(Entity entity) override {
            if (components.contains(entity)) {
                components.remove(entity);
                componentCount--;

                if (entity == lastEntity) {
                    lastEntity = Entity::null();
                    lastComponent = nullptr;
                }
            }
        }

        /**
         * @brief Gets the number of components
         */
        [[nodiscard]] size_t size() const override {
            return componentCount;
        }

        /**
         * @brief Clears all components
         */
        void clear() override {
            components.clear();
            componentCount = 0;
            lastEntity = Entity::null();
            lastComponent = nullptr;
        }

        /**
         * @brief Reserves memory for components
         */
        void reserve(size_t capacity) override {
            components.reserve(capacity);
        }

        /**
         * @brief Minimizes memory usage
         */
        void shrinkToFit() {
            components.shrinkToFit();
        }

        /**
         * @brief Sorts components using a comparison function
         */
        template<typename Compare>
        void sort(Compare compare) {
            components.sort(std::move(compare));
            lastEntity = Entity::null();
            lastComponent = nullptr;
        }

        [[nodiscard]] const SparseSetType& getSparseSet() const { return components; }
        [[nodiscard]] SparseSetType& getSparseSet() { return components; }

        [[nodiscard]] auto begin() noexcept { return components.begin(); }
        [[nodiscard]] auto end() noexcept { return components.end(); }
        [[nodiscard]] auto begin() const noexcept { return components.begin(); }
        [[nodiscard]] auto end() const noexcept { return components.end(); }
        [[nodiscard]] auto cbegin() const noexcept { return components.cbegin(); }
        [[nodiscard]] auto cend() const noexcept { return components.cend(); }

        [[nodiscard]] const std::vector<T, Allocator>& getComponents() const noexcept {
            return components.getComponents();
        }
        [[nodiscard]] std::vector<T, Allocator>& getComponents() noexcept {
            return components.getComponents();
        }

        [[nodiscard]] const auto& getEntities() const noexcept {
            return components.getEntities();
        }
    };
}

#endif