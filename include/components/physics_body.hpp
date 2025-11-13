#ifndef PHYSICS_BODY_H
#define PHYSICS_BODY_H

#include <cstddef>

#include "ctime_typemap.hpp"
#include "entity.hpp"
#include "physics.hpp"
#include "vector.hpp"
#include "transform.hpp"
#include "component_pool.hpp"

class PhysicsBody {
    public:
        explicit PhysicsBody(PhysicsCore& physics, EntityID eid, ComponentPool<Transform>& transform_pool)
        : m_physics (physics)
        , m_eid (eid)
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

template<>
class ComponentPoolTraits<PhysicsBody> {
    template<typename... Ts>
    static void init(ComponentPool<PhysicsBody>& self, TypeMap<Ts...>& pools) {
        pools.template get<Transform>().subscribe_remove_listener(
            [&](EntityID owner) {
                self.remove(owner);
            }
        );
        pools.template get<Transform>().subscribe_swap_listener(
            [&](EntityID owner, size_t new_idx) {
                auto idx = self.find(owner);
                if (idx.has_value()) {
                    self.entry_at(idx.value()).data.transform_idx = new_idx;
                }
            }
        );
    }
};


#endif
