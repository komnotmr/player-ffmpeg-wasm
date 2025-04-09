#include <libavformat/avformat.h>
#include <libavcodec/h264dec.h>

void web_test_function();

void web_test_function() {
    ff_h264_decode_init_vlc();
}