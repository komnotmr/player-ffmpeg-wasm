#include "../test-app/player.hpp"
#include <emscripten.h> // TODO

extern "C" {

    typedef void (*clbck_t)(void *data, size_t len);

    EMSCRIPTEN_KEEPALIVE
    void initialize () noexcept {
        eweb::player::initialize();
    }

    EMSCRIPTEN_KEEPALIVE
    void deinitialize () noexcept {
        eweb::player::deinitialize();
    }

    EMSCRIPTEN_KEEPALIVE
    void push (void *data, size_t len) noexcept {
        eweb::player::push(data, len);
    }

    EMSCRIPTEN_KEEPALIVE
    void process_queue () noexcept {
        eweb::player::process_queue();
    }

    EMSCRIPTEN_KEEPALIVE
    int seek_to_bytes (size_t bytes) noexcept {
        return eweb::player::seek_to_bytes(bytes);
    }

    EMSCRIPTEN_KEEPALIVE
    void on_fragment_video (clbck_t clbck) noexcept {
        eweb::player::on_fragment_video([clbck](eweb::player::fragment_t vfragment){
            clbck(vfragment.data, vfragment.len);
        });
    }

    EMSCRIPTEN_KEEPALIVE
    void on_fragment_audio (clbck_t clbck) noexcept {
        eweb::player::on_fragment_audio([clbck](eweb::player::fragment_t vfragment){
            clbck(vfragment.data, vfragment.len);
        });
    }
}
