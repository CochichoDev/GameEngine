#ifndef PHYSICS_BODY_H
#define PHYSICS_BODY_H

#include <cstddef>

#include "containers/typemap.hpp"
#include "containers/component_pool.hpp"

#include "components/transform.hpp"

#include "entity.hpp"
#include "physics.hpp"
#include "vector.hpp"

struct PhysicsRegistry;

class PhysicsBody {
    public:
        explicit PhysicsBody(PhysicsCore& physics, EntityID eid, size_t transform_idx)
        : m_physics (physics)
        , m_eid (eid)
        , transform_idx (transform_idx)        
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
class ComponentPoolTraits<PhysicsBody, PhysicsRegistry> {
    template<typename... Ts, typename R>
    static void init(ComponentPool<PhysicsBody, PhysicsRegistry>& self, TypeMap<Ts...>& pools) {
        pools.template get<ComponentPool<Transform>>().subscribe_remove_listener(
            [&](EntityID owner) {
                self.remove(owner);
            }
        );
        pools.template get<ComponentPool<Transform>>().subscribe_swap_listener(
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
