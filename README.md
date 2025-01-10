# VECS - Versatile Entity Component System

VECS is a lightweight, high-performance Entity Component System (ECS) framework designed for modern C++. Inspired by EnTT but with a focus on core functionality, VECS prioritizes simplicity and ease of use while maintaining high performance.

## Features

- **Efficient Entity Management**: Uses a compact entity ID system with version control for safe entity recycling
- **Cache-Friendly Component Storage**: Components are stored in contiguous memory using optimized sparse sets
- **Modern C++ Design**: Leverages C++21 features for maximum performance and type safety
- **Memory Efficient**: Minimizes memory overhead and provides predictable memory usage patterns
- **Type Safe**: Full compile-time type checking with no RTTI dependency
- **Header-Only**: Easy to integrate into existing projects
- **Fast Component Access**: O(1) component lookup and entity validation
- **Minimal Dependencies**: Self-contained with only STL dependencies
- **View Support**: Efficient iteration over entities with specific component combinations

## Basic Usage

```cpp
#include "vecs/ECS.h"

// Define components
struct Position { float x, y; };
struct Velocity { float dx, dy; };

// Create ECS instance
vecs::ECS ecs;

// Create entity and add components
auto entity = ecs.createEntity();
ecs.addComponent(entity, Position{0.0f, 0.0f});
ecs.addComponent(entity, Velocity{1.0f, 1.0f});

// Access components
auto& pos = ecs.getComponent<Position>(entity);
auto& vel = ecs.getComponent<Velocity>(entity);

// Update position
pos.x += vel.dx;
pos.y += vel.dy;
```

### Using Views

Views provide an efficient way to iterate over entities that have specific component combinations:

```cpp
// Create multiple entities
for(int i = 0; i < 1000; ++i) {
    auto entity = ecs.createEntity();
    ecs.addComponent(entity, Position{static_cast<float>(i), 0.0f});
    if(i % 2 == 0) {  // Add Velocity only to even-numbered entities
        ecs.addComponent(entity, Velocity{1.0f, 1.0f});
    }
}

// Create a view for entities having both Position and Velocity
auto view = ecs.view<Position, Velocity>();

// Iterate over matching entities and their components
view.each([](const Position& pos, const Velocity& vel) {
    // Process only entities that have both components
});

// You can also get the entity along with its components
view.each([](vecs::Entity entity, Position& pos, const Velocity& vel) {
    // Access entity ID and components
});

// Or iterate over just the entities
view.each([](vecs::Entity entity) {
    // Process entity
});
```

## Performance

VECS has been benchmarked against EnTT, a widely-used ECS framework. Here are the results from our performance tests:

### Benchmark Configuration
- CPU: 28 X 2112 MHz
- CPU Caches:
  - L1 Data: 48 KiB (x14)
  - L1 Instruction: 32 KiB (x14)
  - L2 Unified: 2048 KiB (x14)
  - L3 Unified: 33792 KiB (x1)
- Entity Count: 10,000
- Components: Position (x,y) and Velocity (dx,dy)
- Test: Iteration over all entities with both components
- Build: Release mode with optimizations

### Results

| Metric | Vecs | EnTT |
|--------|------|------|
| Mean Time (μs) | 18.2 | 18.9 |
| Entities/second | 549M | 529M |
| Memory Throughput | 35.14 GB/s | 33.87 GB/s |
| Coefficient of Variation | 0.46% | 0.47% |

Key observations:
- VECS demonstrates superior performance, processing over 1 billion components per second
- 3.7% faster mean execution time compared to EnTT
- Higher memory throughput and better cache utilization
- Excellent stability with marginally better coefficient of variation
- Both frameworks achieve exceptional performance with negligible variance
- Cache-friendly design yields consistent high-performance results

## Performance Considerations

- Components are stored in contiguous arrays for cache-friendly access
- Entity recycling minimizes memory fragmentation
- Sparse sets provide fast component lookup and iteration
- Component pools use cache line alignment for optimal access patterns
- Minimal virtual function usage to reduce overhead
- Template specialization for common component types

## Planned Features

### Short Term
- ✅ **Views**: Efficient iteration over entities with specific component combinations
    - ✅ Single component views
    - ✅ Multi-component views with compile-time filtering
    - Exclusive/inclusive component filters

### Medium Term
- **Groups**: Optimized component access patterns
    - Filter by owned components (must have)
    - Filter by get components (optional)
    - Filter by excluded components (must not have)
    - Group-aware component storage

### Long Term
- **Systems**: Formal system management and execution
    - System dependencies and ordering
    - Parallel system execution
    - System groups and scheduling
- **Events**: Type-safe event emission and handling
- **Serialization**: Component and entity serialization support
- **Multi-threading**: Thread-safe component access and parallel iteration
- **Memory Pools**: Custom allocators for better memory management

## Requirements

- C++21 compatible compiler
- CMake 3.20 or higher (for building tests)
- GoogleTest (for running tests)
- Google Benchmark (for running benchmarks)

## License and Attribution

This project is licensed under the MIT License. When using this code in other projects, proper attribution to the author (Vyxs) must be included. See the LICENSE file for details.