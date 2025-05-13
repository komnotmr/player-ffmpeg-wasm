#include "player.hpp"
#include "renderer.hpp"

#include <chrono>
#include <cstdio>
#include <iostream>
#include <ostream>
#include <thread>

#include <unistd.h>

int main (int argc, char *argv[]) {

    eweb::player::initialize();

    if (!eweb::renderer::initialize()) {
        // return 1;
    }

    // eweb::player::on_fragment_sync(
    //     [](eweb::player::fragment_t vfragment, eweb::player::fragment_t afragment) {
    //         // eweb::renderer::render(vfragment.data, vfragment.len);
    //         // TODO render both video and audio
    //         std::cerr << "got fragment 'video'(" << vfragment.len << ")" << std::endl;
    //         std::cerr << "got fragment 'audio'(" << afragment.len << ")" << std::endl;
    //     }
    // );

    eweb::player::on_fragment_video([](eweb::player::fragment_t fragment){
        std::cerr << "got fragment 'video'(" << fragment.len << ")" << std::endl;
    });

    eweb::player::on_fragment_audio([](eweb::player::fragment_t fragment){
        std::cerr << "got fragment 'audio'(" << fragment.len << ")" << std::endl;
    });

    std::cout << "rendered" << std::endl;

    FILE* f = fopen(argv[1], "rb");
    if (!f) {
        std::cerr << "fopen error " << argv[1] << std::endl;
        return 1;
    }

    while (true) {
        
        uint8_t buf[1024];
        size_t n = fread(buf, 1, sizeof(buf), f);
        std::cout << "red " << n << " bytes" << std::endl;

        if (n <= 0) {
            break;
        }

        eweb::player::push(buf, n);
        eweb::player::process_queue();

        using namespace std::chrono_literals;
        std::cout << "tick" << std::endl;
        std::this_thread::sleep_for(10ms);
    }

    fclose(f);

    eweb::renderer::deinitialize();
    eweb::player::deinitialize();

    return 0;
}
