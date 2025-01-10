//
// Created by Vyxs on 09/01/2025.
//

#ifndef VIEW_H
#define VIEW_H

#include <tuple>
#include "Pool.h"

namespace vecs {
    /**
     * @brief View class for efficient iteration over entities with specific components
     * @tparam Components Component types required in the view
     */
    template<typename... Components>
    class View {
        using ComponentPools = std::tuple<Pool<Components>*...>;
        ComponentPools pools;

        /**
         * @brief Gets the smallest pool among the component pools
         * @return Pointer to the smallest pool
         */
        template<size_t... Indices>
        [[nodiscard]] BasePool* findSmallestPool(std::index_sequence<Indices...>) const noexcept {
            BasePool* smallestPool = nullptr;
            size_t minSize = std::numeric_limits<size_t>::max();

            ([&](BasePool* pool) {
                if (const auto size = pool->size(); size < minSize) {
                    minSize = size;
                    smallestPool = pool;
                }
            }(std::get<Indices>(pools)), ...);

            return smallestPool;
        }

        /**
         * @brief Helper to check if an entity exists in all pools
         */
        [[nodiscard]] bool entityExistsInAllPools(Entity entity) const noexcept {
            return (std::get<Pool<Components>*>(pools)->has(entity) && ...);
        }

    public:
        explicit View(ComponentPools componentPools) noexcept : pools(componentPools) {}

        /**
         * @brief Iterates over entities and their components using a callback function
         * @param function Callback function to process entities/components
         */
        template<typename Func>
        void each(Func&& function) const {
            if constexpr (std::is_invocable_v<Func, Entity, Components&...>) {
                eachEntityWithComponents(std::forward<Func>(function));
            } else if constexpr (std::is_invocable_v<Func, Components&...>) {
                eachComponents(std::forward<Func>(function));
            } else if constexpr (std::is_invocable_v<Func, Entity>) {
                eachEntity(std::forward<Func>(function));
            }
        }

    private:
        /**
         * @brief Iterates over entities and their components
         */
        template<typename Func>
        void eachEntityWithComponents(Func&& function) const {
            const auto* smallestPool = findSmallestPool(std::index_sequence_for<Components...>{});
            if (!smallestPool) return;

            for (const auto& entity : static_cast<const Pool<std::tuple_element_t<0, std::tuple<Components...>>>*>(smallestPool)->getEntities()) {
                if (entityExistsInAllPools(entity)) {
                    function(entity, std::get<Pool<Components>*>(pools)->get(entity)...);
                }
            }
        }

        /**
         * @brief Iterates over components only
         */
        template<typename Func>
        void eachComponents(Func&& function) const {
            const auto* smallestPool = findSmallestPool(std::index_sequence_for<Components...>{});
            if (!smallestPool) return;

            for (const auto& entity : static_cast<const Pool<std::tuple_element_t<0, std::tuple<Components...>>>*>(smallestPool)->getEntities()) {
                if (entityExistsInAllPools(entity)) {
                    function(std::get<Pool<Components>*>(pools)->get(entity)...);
                }
            }
        }

        /**
         * @brief Iterates over entities only
         */
        template<typename Func>
        void eachEntity(Func&& function) const {
            const auto* smallestPool = findSmallestPool(std::index_sequence_for<Components...>{});
            if (!smallestPool) return;

            for (const auto& entity : static_cast<const Pool<std::tuple_element_t<0, std::tuple<Components...>>>*>(smallestPool)->getEntities()) {
                if (entityExistsInAllPools(entity)) {
                    function(entity);
                }
            }
        }
    };
}

#endif