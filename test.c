// // extern "C" {
//     #include <libavcodec/avcodec.h>
//     #include <libavformat/avformat.h>
//     #include <libavutil/imgutils.h>
//     #include <libavutil/avutil.h>
//     #include <libswscale/swscale.h>
// // }

// // #include <iostream>

// int main (int argc, char *argv[]) {

//     // std::string filename = argc > 1 ? argv[1] : nullptr;
//     // if (filename.empty()) {
//     //     filename = "/dev/video0";
//     // }

//     const char *filename = "";

//     AVFormatContext *pFormatContext = avformat_alloc_context();

//     avformat_open_input(&pFormatContext, filename, NULL, NULL);

//     // std::cout
//         // << "OK"
//         // << std::endl;

//     return 0;
// };

#include <libavformat/avformat.h>

void main () {
    av_append_packet(NULL, NULL, 0);
}
