extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/avutil.h>
    #include <libswscale/swscale.h>
}

#include "app.hpp"

#include <iostream>

#define print_and_return(O__,E__) \
    std::O__ << (E__).msg << std::endl; \
    return  (E__).code;


int main (int argc, char *argv[]) {

    app::player player;

    if (!player.from_source(argc > 1 ? argv[1] : "/dev/video0")) {
        print_and_return(cerr, player.get_last_error())
    }

    if (player.initialize_streams()) {
        print_and_return(cerr, player.get_last_error())
    }

    if (!player.start_read_loop()) {
        print_and_return(cerr, player.get_last_error())
    }

    print_and_return(cout, player.get_last_error())
}
