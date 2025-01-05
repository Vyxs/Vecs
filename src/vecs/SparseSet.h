//
// Created by Vyxs on 05/01/2025.
//

#ifndef SPARSESET_H
#define SPARSESET_H

#include <algorithm>
#include <numeric>
#include <vector>
#include <memory>
#include <cassert>
#include "Entity.h"

namespace vecs {
    /**
     * @brief Default allocator for allocating memory for components
     * @tparam T Component type
     */
    template<typename T>
    class DefaultAllocator : public std::allocator<T> {
    public:
        using std::allocator<T>::allocator;

        template<typename U>
        struct rebind {
            using other = DefaultAllocator<U>;
        };
    };

    /**
     * @brief Data structure to manage entities and their associated components efficiently
     * @tparam T Component type
     * @tparam Allocator Allocator type for memory management
     */
    template<typename T, typename Allocator = DefaultAllocator<T>>
    class SparseSet {

        using AllocTraits = std::allocator_traits<Allocator>;
        using EntityAllocator = typename AllocTraits::template rebind_alloc<Entity>;

        std::vector<Entity, EntityAllocator> dense{};
        std::vector<Entity, EntityAllocator> sparse{};
        std::vector<T, Allocator> components{};

        size_t pageSize = 4096;
        static constexpr size_t minCapacity = 64;

        /**
         * @brief Ensures sufficient space for the given entity
         */
        void assureSpace(const Entity entity) {
            const auto entityId = entity.getId();
            if (entityId >= sparse.size()) {
                const auto current = sparse.size();
                const auto required = static_cast<size_t>(entityId) + 1;
                const auto size = required + (required >> 1);

                const auto alignedSize = size + pageSize - 1 & ~(pageSize - 1);
                sparse.resize(alignedSize, Entity::null());

                const auto estimated = std::min(alignedSize, static_cast<size_t>(entityId * 1.2));
                if (estimated > dense.capacity()) {
                    dense.reserve(estimated);
                    components.reserve(estimated);
                }
            }
        }

    public:
        /**
         * @brief Constructs a SparseSet with initial capacity
         */
        explicit SparseSet(const size_t initialCapacity = minCapacity)
            : dense(EntityAllocator{}),
              sparse(EntityAllocator{}),
              components(Allocator{}) {
            reserve(std::max(initialCapacity, minCapacity));
        }

        /**
         * @brief Gets the component associated with an entity
         */
        // ReSharper disable once CppRedundantInlineSpecifier
        [[nodiscard]] inline T& get(const Entity entity) noexcept {
            assert(contains(entity));
            return components[sparse[entity.getId()].getId()];
        }

        /**
         * @brief Gets the component associated with an entity (const)
         */
        // ReSharper disable once CppRedundantInlineSpecifier
        [[nodiscard]] inline const T& get(const Entity entity) const noexcept {
            assert(contains(entity));
            return components[sparse[entity.getId()].getId()];
        }

        /**
         * @brief Checks if an entity has a component
         */
        // ReSharper disable once CppRedundantInlineSpecifier
        [[nodiscard]] inline bool contains(const Entity entity) const noexcept {
            const auto entityId = entity.getId();
            return entityId < sparse.size() &&
                   sparse[entityId].getId() < dense.size() &&
                   dense[sparse[entityId].getId()] == entity;
        }

        /**
         * @brief Inserts a component for an entity
         */
        void insert(const Entity entity, T&& component) {
            assureSpace(entity);

            if (!contains(entity)) {
                const auto pos = dense.size();
                sparse[entity.getId()] = Entity{static_cast<EntityId>(pos)};
                dense.push_back(entity);
                components.push_back(std::move(component));
            }
        }

        /**
         * @brief Constructs a component in place for an entity
         */
        template<typename... Args>
        [[nodiscard]] T& emplace(const Entity entity, Args&&... args) {
            assureSpace(entity);

            if (!contains(entity)) {
                const auto pos = dense.size();
                sparse[entity.getId()] = Entity{static_cast<EntityId>(pos)};
                dense.push_back(entity);
                return components.emplace_back(std::forward<Args>(args)...);
            }
            return components[sparse[entity.getId()].getId()];
        }

        /**
         * @brief Removes a component from an entity
         */
        void remove(const Entity entity) noexcept {
            if (!contains(entity)) return;

            const auto last = dense.size() - 1;
            const auto entityId = entity.getId();
            const auto index = sparse[entityId].getId();
            const auto lastEntity = dense[last];

            components[index] = std::move(components[last]);
            dense[index] = dense[last];
            sparse[lastEntity.getId()] = Entity{static_cast<EntityId>(index)};
            sparse[entityId] = Entity::null();

            dense.pop_back();
            components.pop_back();
        }

        void reserve(size_t capacity) {
            capacity = (capacity + pageSize - 1) & ~(pageSize - 1);
            sparse.reserve(capacity);
            dense.reserve(capacity);
            components.reserve(capacity);
        }

        void shrinkToFit() noexcept {
            sparse.shrink_to_fit();
            dense.shrink_to_fit();
            components.shrink_to_fit();
        }

        void clear() noexcept {
            const auto currentCapacity = capacity();
            sparse.clear();
            dense.clear();
            components.clear();

            if (currentCapacity >= minCapacity) {
                reserve(currentCapacity);
            }
        }

        /**
         * @brief Sorts components based on a comparison function
         */
        template<typename Compare>
        void sort(Compare compare) {
            const auto length = dense.size();
            if (length <= 1) return;

            std::vector<Entity, EntityAllocator> indices(length);
            std::iota(indices.begin(), indices.end(), 0);

            std::sort(indices.begin(), indices.end(),
                     [this, &compare](const auto lhs, const auto rhs) {
                         return compare(components[lhs.getId()], components[rhs.getId()]);
                     });

            std::vector<T, Allocator> tempComponents;
            std::vector<Entity, EntityAllocator> tempEntities;
            tempComponents.reserve(length);
            tempEntities.reserve(length);

            for (size_t i = 0; i < length; ++i) {
                const auto idx = indices[i].getId();
                tempComponents.push_back(std::move(components[idx]));
                tempEntities.push_back(dense[idx]);
                sparse[dense[idx].getId()] = Entity{static_cast<EntityId>(i)};
            }

            components = std::move(tempComponents);
            dense = std::move(tempEntities);
        }

        [[nodiscard]] constexpr size_t size() const noexcept { return dense.size(); }
        [[nodiscard]] constexpr bool empty() const noexcept { return dense.empty(); }
        [[nodiscard]] constexpr size_t capacity() const noexcept { return components.capacity(); }

        [[nodiscard]] auto begin() noexcept { return components.begin(); }
        [[nodiscard]] auto end() noexcept { return components.end(); }
        [[nodiscard]] auto begin() const noexcept { return components.begin(); }
        [[nodiscard]] auto end() const noexcept { return components.end(); }
        [[nodiscard]] auto cbegin() const noexcept { return components.cbegin(); }
        [[nodiscard]] auto cend() const noexcept { return components.cend(); }

        [[nodiscard]] const std::vector<Entity, EntityAllocator>& getEntities() const noexcept { return dense; }
        [[nodiscard]] std::vector<T, Allocator>& getComponents() noexcept { return components; }
        [[nodiscard]] const std::vector<T, Allocator>& getComponents() const noexcept { return components; }
    };
}

#endif