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

    template<typename T, typename Allocator = DefaultAllocator<T>>
    class Pool final : public BasePool {
        using SparseSetType = SparseSet<T, Allocator>;
        SparseSetType components;

    public:
        Pool() = default;

        void insert(Entity entity, T&& component) {
            components.insert(entity, std::forward<T>(component));
        }

        template<typename... Args>
        T& emplace(Entity entity, Args&&... args) {
            return components.emplace(entity, std::forward<Args>(args)...);
        }

        [[nodiscard]] inline T& get(Entity entity) noexcept {
            return components.get(entity);
        }

        [[nodiscard]] inline const T& get(Entity entity) const noexcept {
            return components.get(entity);
        }

        [[nodiscard]] inline bool has(Entity entity) const noexcept {
            return components.contains(entity);
        }

        void removeEntity(Entity entity) override {
            components.remove(entity);
        }

        void clear() override {
            components.clear();
        }

        [[nodiscard]] size_t size() const override {
            return components.size();
        }

        void reserve(size_t capacity) override {
            components.reserve(capacity);
        }

        template<typename Compare>
        void sort(Compare compare) {
            components.sort(std::move(compare));
        }

        [[nodiscard]] const auto& getSparseSet() const noexcept { return components; }
        [[nodiscard]] auto& getSparseSet() noexcept { return components; }
        [[nodiscard]] const auto& getEntities() const noexcept { return components.getEntities(); }
        [[nodiscard]] const auto& getComponents() const noexcept { return components.getComponents(); }
        [[nodiscard]] auto& getComponents() noexcept { return components.getComponents(); }

        [[nodiscard]] auto begin() noexcept { return components.begin(); }
        [[nodiscard]] auto end() noexcept { return components.end(); }
        [[nodiscard]] auto begin() const noexcept { return components.begin(); }
        [[nodiscard]] auto end() const noexcept { return components.end(); }
        [[nodiscard]] auto cbegin() const noexcept { return components.cbegin(); }
        [[nodiscard]] auto cend() const noexcept { return components.cend(); }
    };
}

#endif