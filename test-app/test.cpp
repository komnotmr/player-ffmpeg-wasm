#include "player.hpp"
#include "renderer.hpp"

#include <iostream>
#include <ostream>

int main (int argc, char *argv[]) {

    eweb::player::initialize();

    if (!eweb::renderer::initialize()) {
        return 1;
    }

    eweb::player::on_fragment_sync(
        [](eweb::player::fragment_t vfragment, eweb::player::fragment_t afragment) {
            eweb::renderer::render(vfragment.data, vfragment.len);
            // TODO render both video and audio
            std::cerr << "got fragment 'video'(" << vfragment.len << ")" << std::endl;
            std::cerr << "got fragment 'audio'(" << afragment.len << ")" << std::endl;
        }
    );

    std::cout << "rendered" << std::endl;



    while (eweb::renderer::loop()) {

        eweb::player::process_queue();


        eweb::player::fragment_t mock_fragment;
        eweb::player::push(nullptr, 0);

        // eweb::player::seek_to_bytes(10);
        // eweb::player::seek_to_bytes(-10);

        // stream_cycle_channel(cur_stream, AVMEDIA_TYPE_AUDIO);
        // stream_cycle_channel(cur_stream, AVMEDIA_TYPE_VIDEO);

    }

    eweb::renderer::deinitialize();
    eweb::player::deinitialize();

    return 0;
}
