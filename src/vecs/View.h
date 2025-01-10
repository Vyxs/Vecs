//
// Created by Vyxs on 09/01/2025.
//
#ifndef VIEW_H
#define VIEW_H

#include <tuple>
#include "Pool.h"

namespace vecs {
    template<typename... Components>
    class View {
        using ComponentPools = std::tuple<Pool<Components>*...>;
        ComponentPools pools;

        [[nodiscard]] BasePool* findSmallestPool() const noexcept {
            BasePool* smallest = nullptr;
            size_t minSize = std::numeric_limits<size_t>::max();

            auto checkPool = [&minSize, &smallest](BasePool* pool) {
                if (const auto size = pool->size(); size < minSize) {
                    minSize = size;
                    smallest = pool;
                }
            };

            (checkPool(std::get<Pool<Components>*>(pools)), ...);
            return smallest;
        }

        template<typename Entity>
        [[nodiscard]] constexpr bool entityExistsInAllPools(Entity entity) const noexcept {
            return (std::get<Pool<Components>*>(pools)->has(entity) && ...);
        }

    public:
        explicit View(ComponentPools componentPools) noexcept
            : pools(componentPools) {}

        template<typename Func>
        void each(Func&& function) const {
            const auto* smallest = static_cast<Pool<std::tuple_element_t<0, std::tuple<Components...>>>*>(findSmallestPool());
            if (!smallest) return;

            const auto& entities = smallest->getEntities();

            if constexpr (std::is_invocable_v<Func, Entity, Components&...>) {
                for (const auto entity : entities) {
                    if (entityExistsInAllPools(entity)) {
                        function(entity, std::get<Pool<Components>*>(pools)->get(entity)...);
                    }
                }
            } else if constexpr (std::is_invocable_v<Func, Components&...>) {
                for (const auto entity : entities) {
                    if (entityExistsInAllPools(entity)) {
                        function(std::get<Pool<Components>*>(pools)->get(entity)...);
                    }
                }
            } else if constexpr (std::is_invocable_v<Func, Entity>) {
                for (const auto entity : entities) {
                    if (entityExistsInAllPools(entity)) {
                        function(entity);
                    }
                }
            }
        }
    };
}

#endif