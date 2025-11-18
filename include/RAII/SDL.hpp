#ifndef SDL_H
#define SDL_H

#include "SDL3/SDL_video.h"
#include <SDL3/SDL.h>

#include <iostream>

class SDL {
    public:
        SDL() {
            if (!SDL_Init(SDL_INIT_VIDEO)) {
                std::cerr << "[ERROR] World::World -> SDL_Init: " << SDL_GetError() << std::endl;
                throw std::runtime_error("Failed to initialiaze SDL");
            }
        }

        SDL(const SDL&) = delete;
        SDL& operator=(const SDL&) = delete;

        SDL(SDL&& other) noexcept = delete;
        SDL& operator=(SDL&&) noexcept = delete;

        ~SDL() {
            SDL_Quit();
        }
};

class SDLWindow {
    public:
        explicit SDLWindow(uint32_t width, uint32_t height) {
            if ((m_window = SDL_CreateWindow("Mini Game Project", width, height, 0)) == nullptr) {
                std::cerr << "[ERROR] World::World -> SDL_Window: " << SDL_GetError() << std::endl;
                throw std::runtime_error("Failed to initialiaze SDL");
            }
        }

        ~SDLWindow() {
            if (m_window != nullptr) {
                SDL_DestroyWindow(m_window);
            }
        }

        SDLWindow(const SDLWindow&) = delete;
        SDLWindow& operator=(const SDLWindow&) = delete;

        SDLWindow(SDLWindow&& other) noexcept {
            m_window = other.m_window;
            other.m_window = nullptr;
        }

        SDLWindow& operator=(SDLWindow&& other) noexcept {
            if (this != &other) {
                m_window = other.m_window;
                other.m_window = nullptr;
            }
            return *this;
        }

        SDL_Window* get() {
            return m_window;
        }

    private:
        SDL_Window* m_window;
};

class SDLRenderer {
    public:
        explicit SDLRenderer(SDL_Window* window) {
            if ((m_renderer = SDL_CreateRenderer(window, nullptr)) == nullptr) {
                std::cerr << "[ERROR] World::World -> SDL_Renderer: " << SDL_GetError() << std::endl;
                throw std::runtime_error("Failed to initialiaze SDL");
            }
        }

        ~SDLRenderer() {
            if (m_renderer != nullptr) {
                SDL_DestroyRenderer(m_renderer);
            }
        }

        SDLRenderer(const SDLRenderer&) = delete;
        SDLRenderer& operator=(const SDLRenderer&) = delete;

        SDLRenderer(SDLRenderer&& other) noexcept {
            m_renderer = other.m_renderer;
            other.m_renderer = nullptr;
        }

        SDLRenderer& operator=(SDLRenderer&& other) noexcept {
            if (this != &other) {
                m_renderer = other.m_renderer;
                other.m_renderer = nullptr;
            }
            return *this;
        }

        SDL_Renderer* get() {
            return m_renderer;
        }

    private:
        SDL_Renderer* m_renderer;
};

#endif
