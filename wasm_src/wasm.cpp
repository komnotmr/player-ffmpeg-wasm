#include "../test-app/player.hpp"

#include <emscripten.h>
#include <emscripten/bind.h>

#include <functional>
#include <iostream>
#include <vector>
#include <cstdint>

namespace {

    /**
        Wrappers for interact with JS from C++
    */

    static void wr_push (emscripten::val arr) {
        auto vec = emscripten::vecFromJSArray<uint8_t>(arr);
        eweb::player::push(vec.data(), vec.size());
    }

    static void wr_clbck_bind_(
        emscripten::val &js_clbck,
        std::function<void (eweb::player::frame_clbck_t const&)> const& player_clbck_setter
    ) {
        player_clbck_setter([js_clbck](eweb::player::fragment_t frag) {
            std::cout << "call clbck len(" << frag.len << ")" << std::endl;
            auto vec = std::vector<uint8_t>();
            for (int i = 0; i < frag.len; i++) {
                vec.push_back(frag.data[i]);
            }

            js_clbck.call<void>("call", 0, (emscripten::val::array(std::move(vec))));
        });
    }

    static void wr_clbck_bind_video (emscripten::val callback) {
        wr_clbck_bind_(
            callback,
            eweb::player::on_fragment_video
        );
    }

    static void wr_clbck_bind_audio (emscripten::val callback) {
        wr_clbck_bind_(
            callback,
            eweb::player::on_fragment_audio);
    }

}

EMSCRIPTEN_BINDINGS(eweb_module) {
    emscripten::function("debug", &eweb::player::debug);

    emscripten::function("initialize", &eweb::player::initialize);
    emscripten::function("deinitialize", &eweb::player::deinitialize);

    emscripten::function("push", &wr_push);

    emscripten::function("process_queue", &eweb::player::process_queue);

    emscripten::function("on_fragment_video", &wr_clbck_bind_video);
    emscripten::function("on_fragment_audio", &wr_clbck_bind_audio);

    emscripten::function("call_clbck_by_name", &eweb::player::call_clbck_by_name); // for test only
}
