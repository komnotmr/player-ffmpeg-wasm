#include "app.hpp"

#include <cstdio>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>

    #include <libavutil/imgutils.h>
    #include <libavutil/avutil.h>
    #include <libavutil/fifo.h>

    #include <libswscale/swscale.h>
    
}

#define LOG(fmt,...) \
    fprintf(stdout, fmt "\n", __VA_ARGS__);

#undef av_err2str
#define av_err2str(errnum) av_make_error_string((char*)__builtin_alloca(AV_ERROR_MAX_STRING_SIZE), AV_ERROR_MAX_STRING_SIZE, errnum)

namespace {

}

namespace eweb{namespace player{

    void initialize () noexcept {

    }

    void push (void *data, size_t len) noexcept {

    }

    void on_fragment_video (frame_clbck_t const& clbck) noexcept {

    }

    void on_fragment_audio (frame_clbck_t const& clbck) noexcept {

    }

    void on_fragment_sync (frame_clbck_t const& clbck) noexcept {

    }

}}
