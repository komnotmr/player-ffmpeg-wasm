#include <cstdint>
#include <functional>
#include <vector>
#include <iostream>
#include <cstring>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#include "player.hpp"

namespace {
    static bool is_debug_enabled = false;
}

#define SLOG(...) do { std::cout << __FILE__ << ":" << __LINE__ << " " << __VA_ARGS__ << std::endl; } while (0)

#define DLOG(...) do { if (is_debug_enabled) { std::cout << __FILE__ << ":" << __LINE__ << " "  << __VA_ARGS__ << std::endl; }} while (0)

namespace eweb{namespace player{

static emscripten::val c_;

void debug (bool on_off) {
    SLOG("debug:(" << on_off << ")");
    is_debug_enabled = on_off;
}

static AVFormatContext *fmt_ctx = nullptr;
static AVIOContext *avio_ctx = nullptr;
static AVCodecContext *video_dec_ctx = nullptr;
static AVCodecContext *audio_dec_ctx = nullptr;
static int video_stream_idx = -1;
static int audio_stream_idx = -1;

static AVStream *video_stream = nullptr;
static AVStream *audio_stream = nullptr;

static std::vector<uint8_t> buffer; // buffer for pushed data
static size_t read_pos = 0;         // current reading position inside buffer

static frame_clbck_t video_callback = [](fragment_t frag){(void)frag; SLOG("default video callback called");};
static frame_clbck_t audio_callback = [](fragment_t frag){(void)frag; SLOG("default audio callback called");};
static sync_clbck_t sync_callback = [](fragment_t frag_a, fragment_t frag_b){(void)frag_a; (void)frag_b; SLOG("default sync callback");};

static SwsContext *sws_ctx = nullptr;
static SwrContext *swr_ctx = nullptr;

void call_clbck_by_name (std::string const& name) {
    std::vector<uint8_t> vec = {1,2,3, 5, 6, 7, 8};
    if (name == "audio" || name == "a") {
        audio_callback(fragment_t{.data = vec.data(), .len = vec.size()});
        return;
    }
    if (name == "video" || name == "v") {
        video_callback(fragment_t{.data = vec.data(), .len = vec.size()});
        return;
    }
    if (name == "both" || name == "b") {
        audio_callback(fragment_t{.data = vec.data(), .len = vec.size()});
        video_callback(fragment_t{.data = vec.data(), .len = vec.size()});
        return;
    }
    SLOG("invalid clbck name '" << name << "', availible: 'audio'(a), 'video'(v), 'both'(b)");

}

// Custom AVIO read callback to read from our internal buffer
static int read_packet(void *opaque, uint8_t *buf, int buf_size) {
    DLOG("read packet");
    size_t available = buffer.size() - read_pos;
    if (available == 0)
        return AVERROR_EOF;

    int to_read = (int)std::min<size_t>(buf_size, available);
    memcpy(buf, buffer.data() + read_pos, to_read);
    read_pos += to_read;
    return to_read;
}

// Seek callback for AVIOContext
static int64_t seek(void *opaque, int64_t offset, int whence) {
    DLOG("seek: offset(" << offset << ") whence(" << whence <<")");

    size_t new_pos = 0;
    switch (whence) {
        case SEEK_SET:
            new_pos = (size_t)offset;
            break;
        case SEEK_CUR:
            new_pos = read_pos + (size_t)offset;
            break;
        case SEEK_END:
            new_pos = buffer.size() + (size_t)offset;
            break;
        case AVSEEK_SIZE:
            return (int64_t)buffer.size();
        default:
            return -1;
    }

    if (new_pos > buffer.size())
        return -1;

    read_pos = new_pos;
    return (int64_t)read_pos;
}

void initialize() noexcept {
    DLOG("initialize");
    av_log_set_level(AV_LOG_DEBUG);
    // av_log_set_level(AV_LOG_QUIET);

    // Clean up if re-initialize called
    if (fmt_ctx) {
        avformat_close_input(&fmt_ctx);
        fmt_ctx = nullptr;
    }
    if (avio_ctx) {
        av_freep(&avio_ctx->buffer);
        avio_context_free(&avio_ctx);
        avio_ctx = nullptr;
    }
    if (video_dec_ctx) {
        avcodec_free_context(&video_dec_ctx);
        video_dec_ctx = nullptr;
    }
    if (audio_dec_ctx) {
        avcodec_free_context(&audio_dec_ctx);
        audio_dec_ctx = nullptr;
    }
    if (sws_ctx) {
        sws_freeContext(sws_ctx);
        sws_ctx = nullptr;

    }
    if (swr_ctx) {
        swr_free(&swr_ctx);
        swr_ctx = nullptr;
    }

    buffer.clear();
    read_pos = 0;

    video_stream_idx = -1;
    audio_stream_idx = -1;

    video_callback = nullptr;
    audio_callback = nullptr;
    sync_callback = nullptr;
}

void deinitialize() noexcept {
    DLOG("deinitialize");

    if (fmt_ctx) {
        avformat_close_input(&fmt_ctx);
        fmt_ctx = nullptr;
    }
    if (avio_ctx) {
        av_freep(&avio_ctx->buffer);
        avio_context_free(&avio_ctx);
        avio_ctx = nullptr;
    }
    if (video_dec_ctx) {
        avcodec_free_context(&video_dec_ctx);
        video_dec_ctx = nullptr;
    }
    if (audio_dec_ctx) {
        avcodec_free_context(&audio_dec_ctx);
        audio_dec_ctx = nullptr;
    }
    if (sws_ctx) {
        sws_freeContext(sws_ctx);
        sws_ctx = nullptr;
    }
    if (swr_ctx) {
        swr_free(&swr_ctx);
        swr_ctx = nullptr;
    }

    buffer.clear();
}

void on_fragment_video(frame_clbck_t const& clbck) noexcept {
    DLOG("on_fragment_video");
    video_callback = clbck;
}

void on_fragment_audio(frame_clbck_t const& clbck) noexcept {
    DLOG("on_fragment_audio");
    audio_callback = clbck;
    clbck({.data = nullptr, .len = 0});
}

void on_fragment_sync(sync_clbck_t const& clbck) noexcept {
    DLOG("on_fragment_sync");
    sync_callback = clbck;
}

void push(void *data, size_t len) noexcept {
    DLOG("push");
    // Append incoming data to buffer
    uint8_t* pData = static_cast<uint8_t*>(data);
    buffer.insert(buffer.end(), pData, pData + len);
}

// Helper to open format context from buffer
static bool open_format_context() {
    DLOG("open_format_context");
    // Allocate buffer for AVIOContext
    constexpr int avio_buffer_size = 4096;
    
    unsigned char* avio_buffer_ptr = (unsigned char*)av_malloc(avio_buffer_size);
    if (!avio_buffer_ptr)
        return false;

    // Create AVIOContext
    avio_ctx = avio_alloc_context(
        avio_buffer_ptr,
        avio_buffer_size,
        0,
        nullptr,
        &read_packet,
        nullptr,
        &seek
    );
    
    if (!avio_ctx)
        return false;

    fmt_ctx = avformat_alloc_context();
    if (!fmt_ctx)
        return false;

    fmt_ctx->pb = avio_ctx;
    
    // Since we are feeding raw data, tell format context that input is seekable
    fmt_ctx->flags |= AVFMT_FLAG_CUSTOM_IO | AVFMT_FLAG_NOBUFFER;

    static AVDictionary *opts = nullptr;
    if (!opts) {
        // av_dict_set(&opts, "fflags", "nobuffer", 0);
        av_dict_set(&opts, "flags", "low_delay", 0);
        // av_dict_set(&opts, "probesize", "32", 0);        // smaller probe size
        // av_dict_set(&opts, "analyzeduration", "0", 0);   // no analyze duration
    }

    // Open input with no filename, use custom IO
    int ret = avformat_open_input(&fmt_ctx, nullptr, nullptr, &opts);
    
    if (ret < 0) {
        char errbuf[128];
        av_strerror(ret, errbuf, sizeof(errbuf));
        std::cerr << "Failed to open input: " << errbuf << "\n";
        return false;
    }

    ret = avformat_find_stream_info(fmt_ctx, nullptr);
    
    if (ret < 0) {
        std::cerr << "Failed to find stream info\n";
        return false;
    }

    // Find video and audio streams
    for (unsigned i=0; i<fmt_ctx->nb_streams; i++) {
        AVStream* stream = fmt_ctx->streams[i];
        AVCodecParameters* codecpar = stream->codecpar;

        if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO && video_stream_idx == -1) {
            video_stream_idx = i;
            video_stream = stream;

            const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
            if (!codec) {
                std::cerr << "Video codec not found\n";
                return false;
            }

            video_dec_ctx = avcodec_alloc_context3(codec);
            if (!video_dec_ctx)
                return false;

            if ((ret=avcodec_parameters_to_context(video_dec_ctx, codecpar)) < 0)
                return false;

            if ((ret=avcodec_open2(video_dec_ctx, codec, nullptr)) < 0)
                return false;

            // Initialize sws context for converting frames to RGB24
            sws_ctx = sws_getContext(
                video_dec_ctx->width,
                video_dec_ctx->height,
                video_dec_ctx->pix_fmt,
                video_dec_ctx->width,
                video_dec_ctx->height,
                AV_PIX_FMT_RGB24,
                SWS_BILINEAR,
                nullptr,
                nullptr,
                nullptr
            );

            if (!sws_ctx)
                return false;

        } else if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO && audio_stream_idx == -1) {
            audio_stream_idx = i;
            audio_stream = stream;

            const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
            if (!codec) {
                std::cerr << "Audio codec not found\n";
                return false;
            }

            audio_dec_ctx = avcodec_alloc_context3(codec);
            if (!audio_dec_ctx)
                return false;

            if ((ret=avcodec_parameters_to_context(audio_dec_ctx, codecpar)) < 0)
                return false;

            if ((ret=avcodec_open2(audio_dec_ctx, codec, nullptr)) < 0)
                return false;

            // Setup resampler to convert audio to S16 stereo 44100Hz
            // swr_ctx = swr_alloc_set_opts2(
            //     nullptr,
            //     AV_CH_LAYOUT_STEREO,
            //     AV_SAMPLE_FMT_S16,
            //     44100,
            //     audio_dec_ctx->ch_layout
            //     audio_dec_ctx->sample_fmt,
            //     audio_dec_ctx->sample_rate,
            //     0,
            //     nullptr
            // );

            // if (!swr_ctx || swr_init(swr_ctx) < 0)
            //     return false;
        }
        
        if (video_stream_idx != -1 && audio_stream_idx != -1)
            break; // both streams found
    }

    if (video_stream_idx == -1 && audio_stream_idx == -1) {
        std::cerr << "No audio or video streams found\n";
        return false;
    }

    return true;
}

// Helper to decode a packet and call callbacks
static void decode_packet(AVPacket* pkt) {
    DLOG("decode_packet");
    int ret;

    if (pkt->stream_index == video_stream_idx && video_dec_ctx) {
        ret = avcodec_send_packet(video_dec_ctx, pkt);
        if (ret < 0)
            return;

        while (ret >= 0) {
            AVFrame* frame = av_frame_alloc();
            ret = avcodec_receive_frame(video_dec_ctx, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                av_frame_free(&frame);
                break;
            } else if (ret < 0) {
                av_frame_free(&frame);
                break;
            }

            // Convert frame to RGB24
            int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, frame->width, frame->height, 1);
            uint8_t* rgb_buffer = (uint8_t*)av_malloc(num_bytes);

            uint8_t* dst_data[4] = { rgb_buffer, nullptr, nullptr, nullptr };
            int dst_linesize[4] = { 3 * frame->width, 0, 0, 0 };

            sws_scale(
                sws_ctx,
                frame->data,
                frame->linesize,
                0,
                frame->height,
                dst_data,
                dst_linesize
            );

            fragment_t frag{ rgb_buffer, static_cast<size_t>(num_bytes) };

            // Call video callback
            {
                if (video_callback)
                    video_callback(frag);

                // If sync callback is set and we have last audio frame with matching PTS call it here
                // For simplicity this example does not store last audio frame for sync.
                // You can extend this by buffering frames and matching pts.
            }

            av_free(rgb_buffer);
            av_frame_free(&frame);
        }
        
    } else if (pkt->stream_index == audio_stream_idx && audio_dec_ctx) {

        ret = avcodec_send_packet(audio_dec_ctx, pkt);
        if (ret < 0)
            return;

        while(ret >= 0){
            AVFrame* frame = av_frame_alloc();
            ret=avcodec_receive_frame(audio_dec_ctx, frame);

            if(ret==AVERROR(EAGAIN)||ret==AVERROR_EOF){
                av_frame_free(&frame);
                break;
            }else if(ret<0){
                av_frame_free(&frame);
                break;
            }

            // Convert audio frame to S16 stereo 44100Hz

            int out_nb_samples = av_rescale_rnd(
                swr_get_delay(swr_ctx, frame->sample_rate) + frame->nb_samples,
                44100,
                frame->sample_rate,
                AV_ROUND_UP);

            int out_channels = 2; // stereo
            int out_buffer_size =
                av_samples_get_buffer_size(
                    nullptr,
                    out_channels,
                    out_nb_samples,
                    AV_SAMPLE_FMT_S16,
                    1);

            uint8_t* out_buffer=(uint8_t*)av_malloc(out_buffer_size);

            uint8_t* out_planes[2] = { out_buffer, nullptr };

            int converted_samples =
                swr_convert(
                    swr_ctx,
                    out_planes,
                    out_nb_samples,
                    (const uint8_t**)frame->data,
                    frame->nb_samples);

            fragment_t frag{ out_buffer, static_cast<size_t>(converted_samples*out_channels*av_get_bytes_per_sample(AV_SAMPLE_FMT_S16)) };

            {
                if(audio_callback)
                    audio_callback(frag);

               // As above for sync callback: this example does not implement full sync

               // You can extend by storing last decoded video/audio frames and calling sync_callback when pts match.
            //    if (sync_callback) { 
            //         sync_callback(...) // TODO !!!
            //    }
           }

           av_free(out_buffer);
           av_frame_free(&frame);
       }
   }
}

void process_queue() noexcept {
    DLOG("process_queue");
   // If format context is not opened yet but we have some data - try open it
   if (!fmt_ctx && !buffer.empty()) {
       read_pos=0; // reset reading position before opening
       if (!open_format_context()) {
           // Could not open format context yet - maybe need more data pushed
           SLOG("open_format_context failed (maybe need more data pushed)");
           return;
       }
   }

   if (!fmt_ctx) {
        SLOG("no input");
       return; // no input
   }

   static bool is_dumped = false;

   // Read packets from format context and decode them
   while(true){
       AVPacket pkt;
       av_init_packet(&pkt);

       int ret=av_read_frame(fmt_ctx,&pkt);

       if(ret<0){
           // EOF or need more data?
           break; 
       }

       decode_packet(&pkt);

       av_packet_unref(&pkt);

       if (!is_dumped) {
        is_dumped = true;
        av_dump_format(fmt_ctx, 0, nullptr, 0);
       }
   }

}

bool seek_to_bytes(size_t bytes) noexcept {
   DLOG("seek_to_bytes: bytes(" << bytes << ")");

   // Check bounds
   if(bytes>=buffer.size())
       return false;

   read_pos=bytes;

   // Reset format context so it re-reads from new position
   if(fmt_ctx){
       avformat_close_input(&fmt_ctx);
       fmt_ctx=nullptr;

       if(avio_ctx){
           av_freep(&avio_ctx->buffer);
           avio_context_free(&avio_ctx);
           avio_ctx=nullptr;
       }
   }

   // Also reset decoders
   if(video_dec_ctx){
       avcodec_flush_buffers(video_dec_ctx);
   }
   if(audio_dec_ctx){
       avcodec_flush_buffers(audio_dec_ctx);
   }

   return true;
}

}} // namespace
