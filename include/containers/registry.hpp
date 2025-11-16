#ifndef REGISTRIES_H
#define REGISTRIES_H

#include <cstddef>
#include <cstdint>

#include "entity.hpp"
#include "component_pool.hpp"

struct ComponentHandle {
    EntityID    owner;
    uint8_t     pool_idx;
    std::size_t comp_idx;
};

struct PhysicsRegistry {
    ComponentPool<ComponentHandle> data;
};

struct RenderRegistry {
    ComponentPool<ComponentHandle> data;
};

#endif
