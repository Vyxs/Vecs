//
// Created by Vyxs on 05/01/2025.
//

#ifndef ENTITY_H
#define ENTITY_H

#include <cstdint>
#include <limits>
#include <queue>
#include <vector>

namespace vecs {
    using EntityId = std::uint32_t;
    using Version = std::uint32_t;

    struct EntityConstants {
        static constexpr std::uint32_t idMask = 0x3FFFFFFF;    // 30 bits for entity ID
        static constexpr std::uint32_t versionMask = 0x3;      // 2 bits for version
        static constexpr std::uint32_t versionShift = 30;      // Version starts at bit 30
        static constexpr std::uint32_t nullEntity = std::numeric_limits<std::uint32_t>::max();
    };

    /**
     * @brief Entity identifier with version for safe entity recycling
     * Uses 30 bits for entity ID and 2 bits for version
     */
    class Entity {
        std::uint32_t identifier;

    public:
        constexpr Entity() noexcept : identifier(EntityConstants::nullEntity) {}
        constexpr explicit Entity(const std::uint32_t id) noexcept : identifier(id) {}
        constexpr Entity(const EntityId id, const Version version) noexcept
            : identifier(version << EntityConstants::versionShift | id & EntityConstants::idMask) {}

        [[nodiscard]] constexpr EntityId getId() const noexcept {
            return identifier & EntityConstants::idMask;
        }

        [[nodiscard]] constexpr Version getVersion() const noexcept {
            return (identifier >> EntityConstants::versionShift) & EntityConstants::versionMask;
        }

        [[nodiscard]] constexpr std::uint32_t getValue() const noexcept {
            return identifier;
        }

        constexpr bool operator==(const Entity& other) const noexcept = default;
        constexpr bool operator!=(const Entity& other) const noexcept = default;

        [[nodiscard]] static constexpr Entity null() noexcept {
            return Entity{EntityConstants::nullEntity};
        }

        struct Hash {
            std::size_t operator()(const Entity& entity) const noexcept {
                return std::hash<std::uint32_t>{}(entity.getValue());
            }
        };
    };

    class EntityManager {
        std::queue<EntityId> recycledIds{};
        std::vector<Version> versions{};
        std::vector<Entity> validEntities{};
        EntityId nextId = 0;
        static constexpr size_t minInitialCapacity = 1024;

    public:
        explicit EntityManager(const size_t initialCapacity = minInitialCapacity) {
            const auto capacity = std::max(initialCapacity, minInitialCapacity);
            versions.reserve(capacity);
            validEntities.reserve(capacity);
        }

        [[nodiscard]] Entity create() noexcept {
            EntityId id;

            if (!recycledIds.empty()) {
                id = recycledIds.front();
                recycledIds.pop();
            } else {
                id = nextId++;
                if (id >= versions.size()) {
                    const size_t newSize = std::max<size_t>(id + 1, versions.size() * 2);
                    versions.resize(newSize, 0);
                }
            }

            const Entity entity{id, versions[id]};
            validEntities.push_back(entity);
            return entity;
        }

        void destroy(const Entity entity) noexcept {
            const auto id = entity.getId();

            if (id >= versions.size() || versions[id] != entity.getVersion()) {
                return;
            }

            if (const auto it = std::ranges::find(validEntities, entity); it != validEntities.end()) {
                *it = validEntities.back();
                validEntities.pop_back();
            }

            versions[id] = versions[id] + 1 & EntityConstants::versionMask;
            recycledIds.push(id);
        }

        [[nodiscard]] bool isValid(const Entity entity) const noexcept {
            const auto id = entity.getId();
            if (id >= versions.size()) return false;
            return versions[id] == entity.getVersion();
        }

        void clear() noexcept {
            recycledIds = std::queue<EntityId>{};
            versions.clear();
            versions.shrink_to_fit();
            validEntities.clear();
            validEntities.shrink_to_fit();
            nextId = 0;
        }

        [[nodiscard]] size_t size() const noexcept {
            return validEntities.size();
        }

        [[nodiscard]] size_t capacity() const noexcept {
            return versions.capacity();
        }

        /**
         * @brief Get the list of currently valid entities
         * @return Reference to the vector of valid entities
         */
        [[nodiscard]] const std::vector<Entity>& getEntities() const noexcept {
            return validEntities;
        }
    };
}

#endif