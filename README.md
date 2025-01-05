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

## Performance Considerations

- Components are stored in contiguous arrays for cache-friendly access
- Entity recycling minimizes memory fragmentation
- Sparse sets provide fast component lookup and iteration
- Component pools use cache line alignment for optimal access patterns
- Minimal virtual function usage to reduce overhead
- Template specialization for common component types

## Planned Features

### Short Term
- **Views**: Efficient iteration over entities with specific component combinations
    - Single component views
    - Multi-component views with compile-time filtering
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

## License and Attribution

This project is licensed under the MIT License. When using this code in other projects, proper attribution to the author (Vyxs) must be included. See the LICENSE file for details.