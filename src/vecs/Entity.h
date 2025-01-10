//
// Created by Vyxs on 05/01/2025.
//
#ifndef ENTITY_H
#define ENTITY_H

#include <cstdint>
#include <vector>
#include <queue>
#include <limits>

namespace vecs {
    using EntityId = std::uint32_t;
    using Version = std::uint32_t;

    struct EntityConstants {
        // 30 bits for entity ID, 2 bits for version
        static constexpr std::uint32_t idMask = 0x3FFFFFFF;
        static constexpr std::uint32_t versionMask = 0x3;
        static constexpr std::uint32_t versionShift = 30;
        static constexpr std::uint32_t nullEntity = std::numeric_limits<std::uint32_t>::max();
    };

    class Entity {
        std::uint32_t identifier;

    public:
        constexpr Entity() noexcept : identifier(EntityConstants::nullEntity) {}
        constexpr explicit Entity(const std::uint32_t id) noexcept : identifier(id) {}

        [[nodiscard]] constexpr EntityId getId() const noexcept {
            return identifier & EntityConstants::idMask;
        }

        [[nodiscard]] constexpr Version getVersion() const noexcept {
            return identifier >> EntityConstants::versionShift & EntityConstants::versionMask;
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

        static constexpr size_t initialCapacity = 8192;
        alignas(64) std::vector<Version> versions;
        alignas(64) std::vector<Entity> validEntities;
        alignas(64) std::vector<EntityId> recycledIds;
        EntityId nextId = 0;

        static constexpr size_t pageSize = 4096;
        static constexpr size_t pageMask = ~(pageSize - 1);

        [[nodiscard]] static inline size_t alignToPage(const size_t size) noexcept {
            return size + pageSize - 1 & pageMask;
        }

    public:
        explicit EntityManager(const size_t requestedCapacity = initialCapacity) {
            const size_t capacity = alignToPage(std::max(requestedCapacity, initialCapacity));
            versions.reserve(capacity);
            validEntities.reserve(capacity);
            recycledIds.reserve(capacity / 4);
        }

        [[nodiscard]] Entity create() noexcept {
            EntityId id;

            if (!recycledIds.empty()) {
                id = recycledIds.back();
                recycledIds.pop_back();
            } else {
                id = nextId++;
                if (id >= versions.size()) {
                    const size_t newSize = alignToPage(std::max<size_t>(id + 1, versions.size() * 2));
                    versions.resize(newSize, 0);
                }
            }

            const Entity entity{versions[id] << EntityConstants::versionShift | id};
            validEntities.push_back(entity);
            return entity;
        }

        void destroy(const Entity entity) noexcept {
            const auto id = entity.getId();
            if (id >= versions.size() || versions[id] != entity.getVersion()) {
                return;
            }

            if (const auto it = std::ranges::find(validEntities, entity);
                it != validEntities.end()) {
                *it = validEntities.back();
                validEntities.pop_back();
            }

            versions[id] = versions[id] + 1 & EntityConstants::versionMask;
            recycledIds.push_back(id);
        }

        [[nodiscard]] bool isValid(const Entity entity) const noexcept {
            const auto id = entity.getId();
            return id < versions.size() && versions[id] == entity.getVersion();
        }

        void clear() noexcept {
            versions.clear();
            validEntities.clear();
            recycledIds.clear();
            nextId = 0;
        }

        [[nodiscard]] size_t size() const noexcept {
            return validEntities.size();
        }

        [[nodiscard]] size_t capacity() const noexcept {
            return versions.capacity();
        }

        [[nodiscard]] const std::vector<Entity>& getValidEntities() const noexcept {
            return validEntities;
        }
    };
}

#endif