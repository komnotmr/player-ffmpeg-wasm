#include "player.hpp"

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

namespace /* constants */ {
    constexpr auto g_queue_size_video = 3;

    constexpr auto g_queue_size_audio = 9;

    constexpr auto g_queue_size_frame = (
        g_queue_size_video > g_queue_size_audio
            ? g_queue_size_video
            : g_queue_size_audio
    );
}

namespace {

    struct clock_t {
        double pts;           /* clock base */
        double pts_drift;     /* clock base minus time at which we updated the clock */
        double last_updated;
        double speed;
        int serial;           /* clock is based on a packet with this serial */
        int paused;
        int *queue_serial;    /* pointer to the current packet queue serial, used for obsolete clock detection */
    };

    struct frame_t {
        AVFrame *frame;
        AVSubtitle sub;
        int serial;
        double pts;           /* presentation timestamp for the frame */
        double duration;      /* estimated duration of the frame */
        int64_t pos;          /* byte position of the frame in the input file */
        int width;
        int height;
        int format;
        AVRational sar;
        int uploaded;
        int flip_v;
    };

    struct queue_packet_t {
        AVFifo *pkt_list;
        int nb_packets;
        int size;
        int64_t duration;
        int abort_request;
        int serial;
    };
    
    struct queue_frame_t {
        frame_t queue[g_queue_size_frame];
        int rindex;
        int windex;
        int size;
        int max_size;
        int keep_last;
        int rindex_shown;
        queue_packet_t *pktq;
    };

    struct decoder_t {
        AVPacket *pkt;
        queue_packet_t *queue;
        AVCodecContext *avctx;
        int pkt_serial;
        int finished;
        int packet_pending;
        int64_t start_pts;
        AVRational start_pts_tb;
        int64_t next_pts;
        AVRational next_pts_tb;
    };

    struct audio_params_t {
        int freq;
        AVChannelLayout av_channel_layout;
        enum AVSampleFormat av_sample_format;
        int frame_size;
        int bytes_per_sec;
    };

    struct state_t {
        const AVInputFormat *av_input_format;
        AVFormatContext *av_input_format_context;

        int seek_req;
        int seek_flags;

        int64_t seek_pos;
        int64_t seek_rel;

        /* video */
        clock_t clock_video;
        queue_frame_t queue_video;
        decoder_t decoder_video;

        /* audio */
        clock_t clock_audio;
        queue_frame_t queue_audio;
        decoder_t decoder_audio;
        double audio_clock;
        int audio_clock_serial;
        double audio_diff_cum; /* used for AV difference average computation */
        double audio_diff_avg_coef;
        double audio_diff_threshold;
        int audio_diff_avg_count;
        AVStream *audio_st;
        queue_packet_t audioq;
        int audio_hw_buf_size;
        uint8_t *audio_buf;
        uint8_t *audio_buf1;
        unsigned int audio_buf_size; /* in bytes */
        unsigned int audio_buf1_size;
        int audio_buf_index; /* in bytes */
        int audio_write_buf_size;
        int audio_volume;
        int muted;
        audio_params_t audio_src;
        audio_params_t audio_filter_src;
        audio_params_t audio_tgt;

        struct SwrContext *swr_ctx;
        int frame_drops_early;
        int frame_drops_late;



    };

}

namespace eweb{namespace player{

    void initialize () noexcept {
        stream_open
        if (frame_queue_init(&is->pictq, &is->videoq, VIDEO_PICTURE_QUEUE_SIZE, 1) < 0)
        goto fail;
        if (frame_queue_init(&is->sampq, &is->audioq, SAMPLE_QUEUE_SIZE, 1) < 0)
        goto fail;

        if (packet_queue_init(&is->videoq) < 0 ||
        packet_queue_init(&is->audioq) < 0 ) goto fail;
        init_clock(&is->vidclk, &is->videoq.serial);
        init_clock(&is->audclk, &is->audioq.serial);

    }

    void push (void *data, size_t len) noexcept {

    }

    void process_queue () noexcept {

    }

    bool seek_to_bytes (size_t bytes) noexcept {
        if (seek_by_bytes) {
            pos = -1;
            if (pos < 0 && cur_stream->video_stream >= 0)
                pos = frame_queue_last_pos(&cur_stream->pictq);
            if (pos < 0 && cur_stream->audio_stream >= 0)
                pos = frame_queue_last_pos(&cur_stream->sampq);
            if (pos < 0)
                pos = avio_tell(cur_stream->ic->pb);
            if (cur_stream->ic->bit_rate)
                incr *= cur_stream->ic->bit_rate / 8.0;
            else
                incr *= 180000.0;
            pos += incr;
            stream_seek(cur_stream, pos, incr, 1);
        }
    }

    void on_fragment_video (frame_clbck_t const& clbck) noexcept {

    }

    void on_fragment_audio (frame_clbck_t const& clbck) noexcept {

    }

    void on_fragment_sync (sync_clbck_t const& clbck) noexcept {

    }

    void deinitialize () noexcept {

    }

}}
