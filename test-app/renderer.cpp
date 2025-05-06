#include "renderer.hpp"

extern "C" {
    #include <SDL2/SDL.h>
}

#include <iostream>

namespace {
    static SDL_Window *window_ = nullptr;
    static bool is_opened_ = false;
    static SDL_Renderer *renderer_ = nullptr;
}

namespace eweb{namespace renderer{

    bool initialize () noexcept {

        if (window_) {

            return true;
        }

        do {
            if (SDL_Init(SDL_INIT_VIDEO) < 0) {
                break;
            }

            if (!SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0")) {
                break;
            }

            window_ = SDL_CreateWindow("test",0,0,640,480,0);
            if (!window_) {
                break;
            }

            renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
            if (!renderer_) {
                break;
            }

            SDL_ShowWindow(window_);
            return true;

        } while (false);

        std::cerr << SDL_GetError() << std::endl;
        return false;
    }

    void deinitialize () noexcept {
        SDL_DestroyWindow(window_);
    }

    bool loop () noexcept {
        SDL_Event e;
        SDL_WaitEvent(&e);

        if(e.type == SDL_QUIT)
        {
            SDL_DestroyRenderer(renderer_);
            SDL_Quit();
            return false;
        }

        SDL_SetRenderDrawColor(renderer_, 0xFF, 0xFF, 0xFF, 0xFF);

        SDL_RenderClear(renderer_);

        SDL_RenderPresent(renderer_);

        return true;
    }

}}
