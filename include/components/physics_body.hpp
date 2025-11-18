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
            , m_valid (true)
            , m_eid (eid)
            , transform_idx (transform_idx)        
        {
            m_physics.add_physics_entity(
                    eid, transform_idx, Vector2D<double>{0,0}, speed, Vector2D<double>{0,0}
            );
        }

        ~PhysicsBody() {
            if (m_valid) 
                m_physics.del_physics_entity(m_eid);
        }

        PhysicsBody(PhysicsBody&& other) noexcept
            : m_physics (other.m_physics) 
            , m_valid (true)
            , m_eid (std::move(other.m_eid))
            , transform_idx (std::move(other.transform_idx))
            , speed (std::move(other.speed))
        {
            other.m_valid = false;
        }
        PhysicsBody& operator=(PhysicsBody&& other) noexcept = delete;

    private:
        PhysicsCore&    m_physics;
        bool        m_valid;
        EntityID    m_eid;

    public:
        size_t      transform_idx;
        Vector2D<double>    speed{0,0};
};

template<>
struct ComponentPoolTraits<PhysicsBody, PhysicsRegistry> {
    template<typename... Ts>
    static void init(ComponentPool<PhysicsBody, PhysicsRegistry>& self, TypeMap<Ts...>& pools) {
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
