#include "app.hpp"
#include "renderer.hpp"

#include <iostream>
#include <ostream>

int main (int argc, char *argv[]) {

    eweb::player::initialize();

    eweb::renderer::initialize();

    eweb::player::on_fragment_sync([](eweb::player::fragment_t fragment) {
        std::cerr << "got fragment 'video'(" << fragment.len << ")" << std::endl;
    });

    std::cout << "rendered" << std::endl;

    while (eweb::renderer::loop()) {

    }

    return 0;
}
