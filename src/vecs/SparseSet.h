//
// Created by Vyxs on 05/01/2025.
//
#ifndef SPARSESET_H
#define SPARSESET_H

#include <vector>
#include <algorithm>
#include <numeric>

#include "Entity.h"

namespace vecs {
    template<typename T>
    class DefaultAllocator : public std::allocator<T> {
    public:
        using std::allocator<T>::allocator;
        template<typename U>
        struct rebind { using other = DefaultAllocator<U>; };
    };

    template<typename T, typename Allocator = DefaultAllocator<T>>
    class SparseSet {
        using EntityAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<Entity>;

        std::vector<Entity, EntityAllocator> sparse;
        std::vector<Entity, EntityAllocator> dense;
        std::vector<T, Allocator> components;

        static constexpr size_t initialSize = 8192;
        static constexpr size_t pageSize = 4096;
        static constexpr size_t growthFactor = 2;

        [[nodiscard]] static constexpr size_t roundUpPow2(size_t n) noexcept {
            if (n == 0) return initialSize;
            n--;
            n |= n >> 1;
            n |= n >> 2;
            n |= n >> 4;
            n |= n >> 8;
            n |= n >> 16;
            if constexpr (sizeof(size_t) == 8) n |= n >> 32;
            return n + 1;
        }

        void reserveAndAlignStorage(const size_t newCapacity) {
            const size_t aligned = roundUpPow2(newCapacity);
            sparse.reserve(aligned);
            dense.reserve(aligned);
            components.reserve(aligned);
        }

    public:
        SparseSet() {
            reserveAndAlignStorage(initialSize);
            sparse.resize(initialSize, Entity::null());
        }

        [[nodiscard]] inline bool contains(Entity entity) const noexcept {
            const auto id = entity.getId();
            return id < sparse.size() &&
                   sparse[id].getId() < dense.size() &&
                   dense[sparse[id].getId()] == entity;
        }

        [[nodiscard]] inline T& get(const Entity entity) noexcept {
            return components[sparse[entity.getId()].getId()];
        }

        [[nodiscard]] inline const T& get(const Entity entity) const noexcept {
            return components[sparse[entity.getId()].getId()];
        }

        void insert(Entity entity, T&& component) {
            const auto entityId = entity.getId();

            if (entityId >= sparse.size()) {
                const auto newSize = roundUpPow2(entityId + 1);
                sparse.resize(newSize, Entity::null());

                if (dense.size() >= dense.capacity() / 2) {
                    reserveAndAlignStorage(dense.capacity() * growthFactor);
                }
            }

            if (!contains(entity)) {
                const auto pos = dense.size();
                sparse[entityId] = Entity{static_cast<EntityId>(pos)};
                dense.push_back(entity);
                components.push_back(std::forward<T>(component));
            }
        }

        template<typename... Args>
        T& emplace(Entity entity, Args&&... args) {
            const auto entityId = entity.getId();

            if (entityId >= sparse.size()) {
                const auto newSize = roundUpPow2(entityId + 1);
                sparse.resize(newSize, Entity::null());

                if (dense.size() >= dense.capacity() / 2) {
                    reserveAndAlignStorage(dense.capacity() * growthFactor);
                }
            }

            if (!contains(entity)) {
                const auto pos = dense.size();
                sparse[entityId] = Entity{static_cast<EntityId>(pos)};
                dense.push_back(entity);
                return components.emplace_back(std::forward<Args>(args)...);
            }

            return components[sparse[entityId].getId()];
        }

        void remove(const Entity entity) noexcept {
            if (!contains(entity)) return;

            const auto entityId = entity.getId();
            const auto denseIndex = sparse[entityId].getId();
            const auto lastIndex = dense.size() - 1;
            const auto lastEntity = dense[lastIndex];

            components[denseIndex] = std::move(components[lastIndex]);
            dense[denseIndex] = dense[lastIndex];
            sparse[lastEntity.getId()] = Entity{static_cast<EntityId>(denseIndex)};
            sparse[entityId] = Entity::null();

            dense.pop_back();
            components.pop_back();
        }

        void clear() noexcept {
            const auto sparseSize = sparse.size();
            sparse.clear();
            sparse.resize(sparseSize, Entity::null());
            dense.clear();
            components.clear();
        }

        void reserve(const size_t capacity) {
            reserveAndAlignStorage(capacity);
            sparse.resize(roundUpPow2(capacity), Entity::null());
        }

        template<typename Compare>
        void sort(Compare compare) {
            if (dense.size() <= 1) return;

            std::vector<size_t> indices(dense.size());
            std::iota(indices.begin(), indices.end(), 0);

            std::sort(indices.begin(), indices.end(),
                [&](size_t a, size_t b) {
                    return compare(components[a], components[b]);
                });

            std::vector<T, Allocator> sortedComponents;
            std::vector<Entity, EntityAllocator> sortedDense;
            sortedComponents.reserve(components.size());
            sortedDense.reserve(dense.size());

            for (auto idx : indices) {
                sortedComponents.push_back(std::move(components[idx]));
                sortedDense.push_back(dense[idx]);
                sparse[dense[idx].getId()] = Entity{static_cast<EntityId>(sortedDense.size() - 1)};
            }

            components = std::move(sortedComponents);
            dense = std::move(sortedDense);
        }

        [[nodiscard]] constexpr size_t size() const noexcept { return dense.size(); }
        [[nodiscard]] constexpr bool empty() const noexcept { return dense.empty(); }

        [[nodiscard]] const auto& getEntities() const noexcept { return dense; }
        [[nodiscard]] const auto& getComponents() const noexcept { return components; }
        [[nodiscard]] auto& getComponents() noexcept { return components; }

        [[nodiscard]] auto begin() noexcept { return components.begin(); }
        [[nodiscard]] auto end() noexcept { return components.end(); }
        [[nodiscard]] auto begin() const noexcept { return components.begin(); }
        [[nodiscard]] auto end() const noexcept { return components.end(); }
        [[nodiscard]] auto cbegin() const noexcept { return components.cbegin(); }
        [[nodiscard]] auto cend() const noexcept { return components.cend(); }
    };
}

#endif