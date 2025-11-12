#ifndef WORLD_H
#define WORLD_H

#include <atomic>
#include <thread>

#include "components/physics_body.hpp"
#include "components/transform.hpp"
#include "components/drawable_rect.hpp"

#include "RAII/SDL.hpp"
#include "physics.hpp"
#include "component_pool.hpp"
#include "renderer.hpp"

class World {
    public:
        explicit World(double width, double height)
        : m_sdl_instance    (SDL())
        , m_sdl_window      (SDLWindow(width, height))
        , m_renderer        (&m_sdl_window)
        {
            m_running.store(false, std::memory_order_relaxed);
        }

        ~World() = default;

        PhysicsCore& get_physics_ref() noexcept { return m_physics; };

        void run() {
            m_running.store(true, std::memory_order_relaxed);
            m_world_thread = std::thread(&World::loop, this);
        }

        void add_component(EntityID owner, Transform& transform) {
            m_c_transform.add(owner, transform);
        }
        void add_component(EntityID owner, Drawable& drawable) {
            m_c_drawables.add(owner, drawable);
        }
        void add_component(EntityID owner, PhysicsBody& body) {
            m_c_physics.add(owner, body);
        }

    private:
        void loop() {
            auto next = std::chrono::steady_clock::now();
            while (m_running.load(std::memory_order_relaxed)) {
                /* Recover last recorded physics snapshot and update transforms */
                process_physics_snapshot();

                /* Build all the render commands */
                publish_render_commands();

                next += m_period;
                std::this_thread::sleep_until(next);
            }
        }

        void process_physics_snapshot() {
            uint32_t tick{0};
            do {
                const std::vector<PhysicsSnapshot>& last_snapshot = m_physics.get_last_snapshot_ref(tick);
                
                for (size_t idx = 0; idx < last_snapshot.size(); ++idx) {
                    const PhysicsSnapshot& snap = last_snapshot[idx];
                    auto& transform_c = m_c_transform.entry_at(snap.transform_idx);
                    auto& physics_c = m_c_physics.entry_at(idx);
                    /* Update the position of the Entity */
                    if (snap.id == transform_c.owner) {
                        transform_c.data.value = snap.pos;
                    }
                    /* Update the physics component with snapshot data */
                    if (snap.id == physics_c.owner) {
                        physics_c.data.speed = snap.speed;
                    }
                }
            } while (!m_physics.verify_snapshot_valid(tick));
        }

        void publish_render_commands() {
            std::vector<RenderCommand> render_commands;
            for (auto drawable : m_c_drawables) {
                if (!drawable.push_render_cmd) continue;
                drawable.push_render_cmd(
                        render_commands, 
                        m_c_transform.entry_at(drawable.transform_idx).data);
            }
            m_renderer.publish_frame(std::move(render_commands));
        }

    private:
        std::thread m_world_thread;
        std::atomic<bool> m_running;

        SDL m_sdl_instance;
        SDLWindow   m_sdl_window;

        PhysicsCore m_physics;
        Renderer    m_renderer;

        ComponentPool<Transform>    m_c_transform;
        ComponentPool<PhysicsBody>  m_c_physics;
        ComponentPool<Drawable>     m_c_drawables;
        
        static constexpr double m_dt = 1.0 / 60.0;
        
        static constexpr std::chrono::steady_clock::duration m_period =
            std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                std::chrono::duration<double>{m_dt}
            );
};

#endif
