#ifndef PHYSICS_H
#define PHYSICS_H

#include <vector>

#include "entity.hpp"
#include "vector.hpp"

struct PhysicsData {
    Vector2D    pos;
    Vector2D    speed;
    Vector2D    acc;
};

class PhysicsCore {
    public:
        PhysicsCore() = default;


    private:
        std::vector<PhysicsData>    m_data;
        std::vector<EntityID>       m_ids;      // Keep entity id and data separate for SIMD performance
};

#endif
