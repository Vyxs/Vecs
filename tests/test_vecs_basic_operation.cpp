//
// Created by Vyxs on 05/01/2025.
//

#include <gtest/gtest.h>
#include "../src/vecs/ECS.h"

// Test components
struct Position {
    float x, y;

    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

struct Velocity {
    float dx, dy;

    bool operator==(const Velocity& other) const {
        return dx == other.dx && dy == other.dy;
    }
};

struct Health {
    int value;

    bool operator==(const Health& other) const {
        return value == other.value;
    }
};

class ECSTest : public testing::Test {
protected:
    vecs::ECS ecs;

    void SetUp() override {
        ecs.clear();
    }
};

// Entity Creation and Validation Tests
TEST_F(ECSTest, CreateEntityReturnsValidEntity) {
    const auto entity = ecs.createEntity();
    EXPECT_TRUE(ecs.isValid(entity)) << "Newly created entity should be valid";
    EXPECT_EQ(ecs.size(), 1) << "ECS should have exactly one entity";
}

TEST_F(ECSTest, NullEntityIsInvalid) {
    EXPECT_FALSE(ecs.isValid(vecs::Entity::null())) << "Null entity should not be valid";
}

// Entity Destruction Tests
TEST_F(ECSTest, DestroyEntityMakesItInvalid) {
    const auto entity = ecs.createEntity();
    ecs.destroyEntity(entity);
    EXPECT_FALSE(ecs.isValid(entity)) << "Destroyed entity should not be valid";
    EXPECT_EQ(ecs.size(), 0) << "ECS should have no entities after destruction";
}

TEST_F(ECSTest, DestroyEntityRemovesAllComponents) {
    const auto entity = ecs.createEntity();
    ecs.addComponent(entity, Position{1.0f, 2.0f});
    ecs.addComponent(entity, Velocity{3.0f, 4.0f});

    ecs.destroyEntity(entity);
    EXPECT_FALSE(ecs.hasComponent<Position>(entity)) << "Destroyed entity should not have Position component";
    EXPECT_FALSE(ecs.hasComponent<Velocity>(entity)) << "Destroyed entity should not have Velocity component";
}

// Component Addition Tests
TEST_F(ECSTest, AddComponentStoresCorrectData) {
    const auto entity = ecs.createEntity();
    constexpr Position pos{1.0f, 2.0f};
    const auto& storedPos = ecs.addComponent(entity, Position{pos});

    EXPECT_EQ(storedPos, pos) << "Stored component should match added component";
    EXPECT_TRUE(ecs.hasComponent<Position>(entity)) << "Entity should have Position component";
}

TEST_F(ECSTest, AddComponentToInvalidEntityThrows) {
    EXPECT_THROW(ecs.addComponent(vecs::Entity::null(), Position{1.0f, 2.0f}),
                 std::runtime_error) << "Adding component to invalid entity should throw";
}

// Component Emplacement Tests
TEST_F(ECSTest, EmplaceComponentConstructsInPlace) {
    const auto entity = ecs.createEntity();
    const auto& pos = ecs.emplaceComponent<Position>(entity, 1.0f, 2.0f);

    EXPECT_EQ(pos, (Position{1.0f, 2.0f})) << "Emplaced component should be constructed with given arguments";
    EXPECT_TRUE(ecs.hasComponent<Position>(entity)) << "Entity should have Position component";
}

TEST_F(ECSTest, EmplaceComponentToInvalidEntityThrows) {
    EXPECT_THROW(ecs.emplaceComponent<Position>(vecs::Entity::null(), 1.0f, 2.0f),
                 std::runtime_error) << "Emplacing component to invalid entity should throw";
}

// Component Replacement Tests
TEST_F(ECSTest, ReplaceExistingComponentUpdatesData) {
    const auto entity = ecs.createEntity();
    ecs.addComponent(entity, Position{1.0f, 2.0f});

    constexpr Position newPos{3.0f, 4.0f};
    const auto& replacedPos = ecs.replaceComponent(entity, Position{newPos});

    EXPECT_EQ(replacedPos, newPos) << "Replaced component should match new data";
}

TEST_F(ECSTest, ReplaceNonexistentComponentAddsIt) {
    const auto entity = ecs.createEntity();
    constexpr Position pos{1.0f, 2.0f};
    const auto& replacedPos = ecs.replaceComponent(entity, Position{pos});

    EXPECT_EQ(replacedPos, pos) << "Component should be added if it didn't exist";
    EXPECT_TRUE(ecs.hasComponent<Position>(entity)) << "Entity should have Position component";
}

// Component Retrieval Tests
TEST_F(ECSTest, GetComponentReturnsCorrectData) {
    const auto entity = ecs.createEntity();
    constexpr Position pos{1.0f, 2.0f};
    ecs.addComponent(entity, Position{pos});

    const auto& retrievedPos = ecs.getComponent<Position>(entity);
    EXPECT_EQ(retrievedPos, pos) << "Retrieved component should match added component";
}

TEST_F(ECSTest, GetComponentFromInvalidEntityThrows) {
    const auto action = [this]() {
        const auto& component = ecs.getComponent<Position>(vecs::Entity::null());
        (void)component; // Suppress unused variable warning
    };
    EXPECT_THROW(action(), std::runtime_error)
        << "Getting component from invalid entity should throw";
}

// Component Check Tests
TEST_F(ECSTest, HasComponentReturnsTrueForExistingComponent) {
    const auto entity = ecs.createEntity();
    ecs.addComponent(entity, Position{1.0f, 2.0f});

    EXPECT_TRUE(ecs.hasComponent<Position>(entity)) << "hasComponent should return true for existing component";
    EXPECT_FALSE(ecs.hasComponent<Velocity>(entity)) << "hasComponent should return false for non-existing component";
}

TEST_F(ECSTest, HasComponentsChecksMultipleComponents) {
    const auto entity = ecs.createEntity();
    ecs.addComponent(entity, Position{1.0f, 2.0f});
    ecs.addComponent(entity, Velocity{3.0f, 4.0f});

    const bool hasRequiredComponents = ecs.hasComponents<Position, Velocity>(entity);
    EXPECT_TRUE(hasRequiredComponents) << "hasComponents should return true when all components exist";

    const bool hasAllComponents = ecs.hasComponents<Position, Velocity, Health>(entity);
    EXPECT_FALSE(hasAllComponents) << "hasComponents should return false when any component is missing";
}

// Component Removal Tests
TEST_F(ECSTest, RemoveComponentRemovesSpecificComponent) {
    const auto entity = ecs.createEntity();
    ecs.addComponent(entity, Position{1.0f, 2.0f});
    ecs.addComponent(entity, Velocity{3.0f, 4.0f});

    ecs.removeComponent<Position>(entity);

    EXPECT_FALSE(ecs.hasComponent<Position>(entity)) << "Removed component should not exist";
    EXPECT_TRUE(ecs.hasComponent<Velocity>(entity)) << "Other components should remain";
}

TEST_F(ECSTest, RemoveComponentsRemovesMultipleComponents) {
    const auto entity = ecs.createEntity();
    ecs.addComponent(entity, Position{1.0f, 2.0f});
    ecs.addComponent(entity, Velocity{3.0f, 4.0f});
    ecs.addComponent(entity, Health{100});

    ecs.removeComponents<Position, Velocity>(entity);

    EXPECT_FALSE(ecs.hasComponent<Position>(entity)) << "First removed component should not exist";
    EXPECT_FALSE(ecs.hasComponent<Velocity>(entity)) << "Second removed component should not exist";
    EXPECT_TRUE(ecs.hasComponent<Health>(entity)) << "Unremoved component should remain";
}

// Clear Tests
TEST_F(ECSTest, ClearRemovesAllEntitiesAndComponents) {
    const auto entity1 = ecs.createEntity();
    const auto entity2 = ecs.createEntity();
    ecs.addComponent(entity1, Position{1.0f, 2.0f});
    ecs.addComponent(entity2, Velocity{3.0f, 4.0f});

    ecs.clear();

    EXPECT_EQ(ecs.size(), 0) << "ECS should have no entities after clear";
    EXPECT_FALSE(ecs.isValid(entity1)) << "First entity should not be valid after clear";
    EXPECT_FALSE(ecs.isValid(entity2)) << "Second entity should not be valid after clear";
    EXPECT_FALSE(ecs.hasComponent<Position>(entity1)) << "Components should not exist after clear";
    EXPECT_FALSE(ecs.hasComponent<Velocity>(entity2)) << "Components should not exist after clear";
}

// Entity Recycling Tests
TEST_F(ECSTest, DestroyedEntityIdsAreRecycled) {
    const auto entity1 = ecs.createEntity();
    EXPECT_TRUE(ecs.isValid(entity1)) << "Entity should be valid when created";

    ecs.destroyEntity(entity1);
    EXPECT_FALSE(ecs.isValid(entity1)) << "Entity should be invalid after destruction";

    const auto entity2 = ecs.createEntity();
    EXPECT_TRUE(ecs.isValid(entity2)) << "New entity should be valid";
    EXPECT_EQ(entity1.getId(), entity2.getId()) << "Destroyed entity ID should be recycled";
    EXPECT_FALSE(ecs.isValid(entity1)) << "Original entity should remain invalid";
    EXPECT_TRUE(ecs.isValid(entity2)) << "Recycled entity should be valid";
}

// Version Tests
TEST_F(ECSTest, EntityVersionIsIncrementedOnDestroy) {
    const auto entity1 = ecs.createEntity();
    const auto version1 = entity1.getVersion();

    ecs.destroyEntity(entity1);
    const auto entity2 = ecs.createEntity();

    EXPECT_EQ(entity1.getId(), entity2.getId()) << "IDs should match for recycled entity";
    EXPECT_NE(entity2.getVersion(), version1) << "Version should be incremented";
}

TEST_F(ECSTest, VersionWrapsAround) {
    auto entity = ecs.createEntity();

    for(int i = 0; i < 5; ++i) {  // More than version bits (2 bits = 4 versions)
        ecs.destroyEntity(entity);
        entity = ecs.createEntity();
    }

    const auto finalVersion = entity.getVersion();
    EXPECT_LE(finalVersion, vecs::EntityConstants::versionMask)
        << "Version should wrap around within mask limits";
}