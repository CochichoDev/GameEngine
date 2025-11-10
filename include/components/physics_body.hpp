#ifndef PHYSICS_BODY
#define PHYSICS_BODY

#include <cstddef>

#include "../entity.hpp"
#include "../physics.hpp"
#include "../world.hpp"

class PhysicsBody {
    public:
        explicit PhysicsBody(World& world, EntityID eid, size_t transform_idx)
        : m_world (world)
        , m_eid (eid)
        , m_transform_idx (transform_idx)
        {
            m_world.get_physics_ref().add_physics_entity(
                    eid, Vector2D{0,0}, Vector2D{0,0}, Vector2D{0,0}
            );
        }

        ~PhysicsBody() {
            m_world.get_physics_ref().del_physics_entity(m_eid);
        }

    private:
        World&      m_world;

        EntityID    m_eid;
        size_t      m_transform_idx;
};

#endif
