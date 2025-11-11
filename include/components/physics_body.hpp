#ifndef PHYSICS_BODY_H
#define PHYSICS_BODY_H

#include <cstddef>

#include "entity.hpp"
#include "physics.hpp"
#include "vector2d.hpp"

class PhysicsBody {
    public:
        explicit PhysicsBody(PhysicsCore& physics, EntityID eid, size_t transform_glb_idx)
        : m_physics (physics)
        , m_eid (eid)
        , transform_idx (transform_glb_idx)
        {
            m_physics.add_physics_entity(
                    eid, transform_idx, Vector2D{0,0}, speed, Vector2D{0,0}
            );
        }

        ~PhysicsBody() {
            m_physics.del_physics_entity(m_eid);
        }

    private:
        PhysicsCore&    m_physics;
        EntityID    m_eid;

    public:
        size_t      transform_idx;
        Vector2D    speed{0,0};
};

#endif
