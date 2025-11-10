#ifndef WORLD_H
#define WORLD_H

#include <thread>

#include "physics.hpp"

class World {
    public:
        World() = default;
        ~World() = default;

        PhysicsCore& get_physics_ref() noexcept { return m_physics; };


    private:
        std::thread m_world_thread;
        
        PhysicsCore m_physics;
};

#endif
