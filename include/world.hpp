#ifndef WORLD_H
#define WORLD_H

#include <atomic>
#include <thread>

#include "containers/typemap.hpp"
#include "containers/component_pool.hpp"
#include "containers/registry.hpp"

#include "components/physics_body.hpp"
#include "components/transform.hpp"
#include "components/drawable_rect.hpp"

#include "RAII/SDL.hpp"
#include "physics.hpp"
#include "renderer.hpp"

class World {
    public:
        explicit World(double width, double height)
        : m_sdl_instance    (SDL())
        , m_sdl_window      (SDLWindow(width, height))
        , m_renderer        (&m_sdl_window)
        {
            m_running.store(false, std::memory_order_relaxed);

            m_pools.for_each([this](auto& pool) {
                pool.init(this->m_pools);
            });
        }

        ~World() = default;

        PhysicsCore& get_physics_ref() noexcept { return m_physics; };

        void run() {
            m_running.store(true, std::memory_order_relaxed);
            m_world_thread = std::thread(&World::loop, this);
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
                    auto& transform_c = m_pools.get<ComponentPool<Transform>>().entry_at(snap.transform_idx);
                    /* Update the position of the Entity */
                    if (snap.id == transform_c.owner) {
                        transform_c.data.value = snap.pos;
                    }
                }
            } while (!m_physics.verify_snapshot_valid(tick));
        }

        void publish_render_commands() {
            std::vector<RenderCommand> render_commands;
            for (auto& entry : m_render_reg.data) {
                auto& renderable = entry.data;
                m_pools.for_index(renderable.pool_idx, [this, &renderable, &render_commands]<typename Pool>(Pool& pool) {
                    using Comp = typename Pool::value_type;

                    if constexpr (requires (Comp& c, std::vector<RenderCommand>& rc, Transform& t) {
                        c.build_render_cmd(rc, t);
                    }) {
                        auto& comp = pool.entry_at(renderable.comp_idx).data;
                        comp.build_render_cmd(
                                render_commands,
                                this->m_pools.get<ComponentPool<Transform>>().entry_at(comp.transform_idx));
                    }
                });
            }
            m_renderer.publish_frame(std::move(render_commands));
        }

    private:
        using VariantPool = std::variant<
            ComponentPool<Transform>*,
            ComponentPool<PhysicsBody, PhysicsRegistry>*,
            ComponentPool<RectangleDrawable, RenderRegistry>*
        >;

    private:
        std::thread m_world_thread;
        std::atomic<bool> m_running;

        SDL m_sdl_instance;
        SDLWindow   m_sdl_window;

        PhysicsCore m_physics;
        Renderer    m_renderer;

        PhysicsRegistry m_physics_reg;
        RenderRegistry  m_render_reg;

        TypeMap<
            ComponentPool<Transform>,
            ComponentPool<PhysicsBody, PhysicsRegistry>,
            ComponentPool<RectangleDrawable, RenderRegistry>
        > m_pools{
            ComponentPool<Transform>{},
            ComponentPool<PhysicsBody, PhysicsRegistry>{&m_physics_reg},
            ComponentPool<RectangleDrawable, RenderRegistry>{&m_render_reg}
        };

        static constexpr double m_dt = 1.0 / 60.0;
        
        static constexpr std::chrono::steady_clock::duration m_period =
            std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                std::chrono::duration<double>{m_dt}
            );
};

#endif
