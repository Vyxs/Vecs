//
// Created by Vyxs on 09/01/2025.
//

#include <gtest/gtest.h>
#include "../src/vecs/ECS.h"
#include "../src/vecs/View.h"

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

class ViewTest : public testing::Test {
protected:
    vecs::ECS ecs;

    void SetUp() override {
        ecs.clear();
    }
};

TEST_F(ViewTest, CreateViewWithSingleComponent) {
    const auto entity = ecs.createEntity();
    ecs.addComponent(entity, Position{1.0f, 2.0f});

    int count = 0;
    const auto view = ecs.view<Position>();
    view.each([&count](const Position& pos) {
        EXPECT_EQ(pos.x, 1.0f);
        EXPECT_EQ(pos.y, 2.0f);
        count++;
    });

    EXPECT_EQ(count, 1);
}

TEST_F(ViewTest, ViewWithMultipleComponents) {
    const auto entity = ecs.createEntity();
    ecs.addComponent(entity, Position{1.0f, 2.0f});
    ecs.addComponent(entity, Velocity{3.0f, 4.0f});

    int count = 0;
    const auto view = ecs.view<Position, Velocity>();
    view.each([&count](const Position& pos, const Velocity& vel) {
        EXPECT_EQ(pos.x, 1.0f);
        EXPECT_EQ(pos.y, 2.0f);
        EXPECT_EQ(vel.dx, 3.0f);
        EXPECT_EQ(vel.dy, 4.0f);
        count++;
    });

    EXPECT_EQ(count, 1);
}

TEST_F(ViewTest, ViewWithEntityOnly) {
    const auto entity = ecs.createEntity();
    ecs.addComponent(entity, Position{1.0f, 2.0f});
    ecs.addComponent(entity, Velocity{3.0f, 4.0f});

    int count = 0;
    const auto view = ecs.view<Position, Velocity>();
    view.each([&count, entity](vecs::Entity e) {
        EXPECT_EQ(e, entity);
        count++;
    });

    EXPECT_EQ(count, 1);
}

TEST_F(ViewTest, ModifyComponentsViaView) {
    const auto entity = ecs.createEntity();
    ecs.addComponent(entity, Position{1.0f, 2.0f});

    const auto view = ecs.view<Position>();
    view.each([](Position& pos) {
        pos.x = 5.0f;
        pos.y = 6.0f;
    });

    const auto& pos = ecs.getComponent<Position>(entity);
    EXPECT_EQ(pos.x, 5.0f);
    EXPECT_EQ(pos.y, 6.0f);
}

TEST_F(ViewTest, EmptyViewIteratesNothing) {
    const auto view = ecs.view<Position>();
    int count = 0;
    view.each([&count](const Position&) {
        count++;
    });

    EXPECT_EQ(count, 0);
}

TEST_F(ViewTest, ViewIteratesMultipleEntities) {
    const auto entity1 = ecs.createEntity();
    const auto entity2 = ecs.createEntity();
    ecs.addComponent(entity1, Position{1.0f, 2.0f});
    ecs.addComponent(entity2, Position{3.0f, 4.0f});

    int count = 0;
    const auto view = ecs.view<Position>();
    view.each([&count](const Position&) {
        count++;
    });

    EXPECT_EQ(count, 2);
}

TEST_F(ViewTest, ViewSkipsEntitiesWithoutAllComponents) {
    const auto entity1 = ecs.createEntity();
    const auto entity2 = ecs.createEntity();
    ecs.addComponent(entity1, Position{1.0f, 2.0f});
    ecs.addComponent(entity1, Velocity{3.0f, 4.0f});
    ecs.addComponent(entity2, Position{5.0f, 6.0f});

    int count = 0;
    const auto view = ecs.view<Position, Velocity>();
    view.each([&count](const Position&, const Velocity&) {
        count++;
    });

    EXPECT_EQ(count, 1);
}

TEST_F(ViewTest, ViewPerformanceTest) {
    constexpr int entityCount = 10000;

    for (int i = 0; i < entityCount; ++i) {
        const auto entity = ecs.createEntity();
        ecs.addComponent(entity, Position{static_cast<float>(i), static_cast<float>(i)});
        if (i % 2 == 0) {
            ecs.addComponent(entity, Velocity{1.0f, 1.0f});
        }
    }

    int count = 0;
    const auto view = ecs.view<Position, Velocity>();
    view.each([&count](const Position&, const Velocity&) {
        count++;
    });

    EXPECT_EQ(count, entityCount / 2);
}