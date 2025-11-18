#ifndef RENDERER_H
#define RENDERER_H

#include <thread>
#include <optional>

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_rect.h"

#include "SDL3/SDL_video.h"
#include "containers/triple_buffer.hpp"

#include "vector.hpp"
#include "colors.hpp"
#include "RAII/SDL.hpp"

#include "SDL3/SDL_render.h"

struct RenderCommand {
    Vector2D<double> pos;
    Vector2D<double> size;
    Vector3D<double> color;
};

class Renderer {
    public:
        explicit Renderer()
        : m_sdl_instance (SDL())
        , m_sdl_window (480, 240)
        {
            m_running.store(false, std::memory_order_relaxed);
        }
        ~Renderer() {
            m_running.store(false, std::memory_order_relaxed);
            if (m_render_thread.joinable()) m_render_thread.join();
        }

        void run() {
            m_running.store(true, std::memory_order_relaxed);
            m_render_thread = std::thread(&Renderer::init, this);
        }

        void publish_frame(const std::vector<RenderCommand>& new_frame) {
            m_cmds.produce(new_frame);
        }
        void publish_frame(std::vector<RenderCommand>&& new_frame) {
            m_cmds.produce(std::move(new_frame));
        }

        bool poll_event(SDL_Event* event) {
            return SDL_PollEvent(event);
        }

    private:
        void init() {
            m_sdl_renderer = new SDLRenderer(m_sdl_window.get());
            loop();
        }

        void loop() {
            auto next = std::chrono::steady_clock::now();
            while (m_running.load(std::memory_order_relaxed)) {
                /* Consume all render commands */
                const auto [data, new_frame] = m_cmds.consume();

                if (new_frame) {
                    SDL_SetRenderDrawColor(
                            m_sdl_renderer->get(), 
                            color::blue_cornflower.x, color::blue_cornflower.y, color::blue_cornflower.z, 
                            SDL_ALPHA_OPAQUE);
                    SDL_RenderClear(m_sdl_renderer->get());
                    
                    for (auto cmd : data) {
                        SDL_SetRenderDrawColor(
                                m_sdl_renderer->get(), 
                                cmd.color.x, cmd.color.y, cmd.color.z, 
                                SDL_ALPHA_OPAQUE);

                        SDL_FRect rect{
                            static_cast<float>(cmd.pos.x), 
                            static_cast<float>(cmd.pos.y), 
                            static_cast<float>(cmd.size.x), 
                            static_cast<float>(cmd.size.y)
                        };
                        SDL_RenderFillRect(m_sdl_renderer->get(), &rect);
                    }
                    
                    SDL_RenderPresent(m_sdl_renderer->get());
                }
                next += m_period;
                std::this_thread::sleep_until(next);
            }
        }
    private:
        SDL m_sdl_instance;
        SDLWindow   m_sdl_window;
        SDLRenderer* m_sdl_renderer{nullptr};

        std::atomic<bool>   m_running;
        std::thread m_render_thread;

        TripleBuffer<RenderCommand> m_cmds;

        static constexpr double m_dt = 1.0 / 60.0;
        
        static constexpr std::chrono::steady_clock::duration m_period =
            std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                std::chrono::duration<double>{m_dt}
            );
};

#endif
