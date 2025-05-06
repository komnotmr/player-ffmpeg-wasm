#include "app.hpp"

#include <cstdio>
#define LOG(fmt,...) \
    fprintf(stdout, fmt "\n", __VA_ARGS__);

#undef av_err2str
#define av_err2str(errnum) av_make_error_string((char*)__builtin_alloca(AV_ERROR_MAX_STRING_SIZE), AV_ERROR_MAX_STRING_SIZE, errnum)

namespace app{

    player::error::error (uint32_t code, std::string const& msg) :
        msg(msg),
        code(code)
        { }

    stream_ctx::~stream_ctx () {
        if (this->codec_context) {
            avcodec_free_context(&this->codec_context);
            this->codec_context = nullptr;
        }
    }

    void audio_stream_ctx::dump_info () {
        LOG("Codec '%s':", this->name_.c_str());
        LOG("\t%d channels, sample rate %d", this->codec_params->ch_layout.nb_channels, this->codec_params->sample_rate);
    }

    void video_stream_ctx::dump_info () {
        LOG("Codec '%s':", this->name_.c_str());
        LOG("\tresolution %d x %d", this->codec_params->width, this->codec_params->height);
    }

    player::error::error (std::string const& msg) :
        player::error::error (1, msg)
        { }

    player::player () :
        format_ctx_(nullptr),
        video_stream_ctx_(video_stream_ctx{}),
        audio_stream_ctx_(audio_stream_ctx{}),
        last_error_(0, "success") {
            streams_ = {
                {video_stream_ctx_.type, &video_stream_ctx_},
                {audio_stream_ctx_.type, &audio_stream_ctx_},
            };
        }

    player::~player () {
        if (this->format_ctx_) {
            avformat_close_input(&this->format_ctx_);
            avformat_free_context(this->format_ctx_);
            this->format_ctx_ = nullptr;
        }
    }

    bool player::fail_ (std::string const& msg) {
        this->last_error_ = error{msg};
        return false;
    }

    bool player::from_source (std::string const& source) {
        if (source.empty()) {
            return this->fail_("source is empty");
        }
        
        source_ = source;
        LOG("source: %s", (source_.c_str()));

        this->format_ctx_ = avformat_alloc_context();
        if (!this->format_ctx_) {
            return this->fail_("call avformat_alloc_context failed");
        }

        #define ff_operation__(operation_name,...) do { \
            LOG("execute operation: %s", #operation_name) \
            auto ret = operation_name(__VA_ARGS__); \
            if (ret) { \
                return this->fail_(av_err2str(ret)); \
            } \
            } while (0);

        ff_operation__(avformat_open_input, &this->format_ctx_, source_.c_str(), nullptr, nullptr);

        ff_operation__(avformat_find_stream_info, this->format_ctx_, nullptr);

        #undef ff_operation__

        av_dump_format(this->format_ctx_, 0, source_.c_str(), 0);       

        return true;
    }

    bool player::initialize_streams () {
        if (!this->format_ctx_) {
            return fail_("format_ctx_ is not initialized");
        }

        LOG("streams count: %d", this->format_ctx_->nb_streams);

        for (int8_t i = 0; i < this->format_ctx_->nb_streams; i++) {
            auto istream = this->format_ctx_->streams[i];
            auto stream_ctx = this->streams_.find(istream->codecpar->codec_type);
            LOG("stream type(%d), istream type(%d), (V%d, A%d)", stream_ctx->second->type, istream->codecpar->codec_type, AVMEDIA_TYPE_VIDEO, AVMEDIA_TYPE_AUDIO);
            if (stream_ctx != this->streams_.end()) {
                stream_ctx->second->codec_params = istream->codecpar;
                stream_ctx->second->stream = istream;
                stream_ctx->second->codec = const_cast<AVCodec *>
                    (avcodec_find_decoder(istream->codecpar->codec_id));
                if (stream_ctx->second->codec) {

                    stream_ctx->second->codec_context = avcodec_alloc_context3(stream_ctx->second->codec);
                    if (!stream_ctx->second->codec_context) {
                        return this->fail_("avcodec_alloc_context3 failed");
                    }

                    if (avcodec_parameters_to_context(stream_ctx->second->codec_context, stream_ctx->second->codec_params) < 0) {
                        return this->fail_("avcodec_parameters_to_context failed");
                    }

                    if (avcodec_open2(stream_ctx->second->codec_context, stream_ctx->second->codec, nullptr) < 0) {
                        return this->fail_("avcodec_parameters_to_context failed");
                    }

                    stream_ctx->second->initialized = true;
                }
            }
        }

        for (auto const& ctx: this->streams_) {
            if (ctx.second->initialized) {
                ctx.second->dump_info();
            }
        }

        return (this->audio_stream_ctx_.initialized || this->video_stream_ctx_.initialized)
            ? true
            : this->fail_("video and audio streams not found");
    }

    bool player::start_read_loop () {
        AVPacket *packet = av_packet_alloc();
        AVFrame *frame = av_frame_alloc();
        while (av_read_frame(this->format_ctx_, packet) >= 0) {
            // TODO: test video or audio
            avcodec_send_packet(this->video_stream_ctx_.codec_context, packet);
            avcodec_receive_frame(this->video_stream_ctx_.codec_context, frame);
            LOG(
                "Frame %c (%d) pts %d dts %d key_frame %d",
                av_get_picture_type_char(frame->pict_type),
                this->video_stream_ctx_.codec_context->frame_num,
                frame->pts,
                frame->pkt_dts,
                frame->key_frame
            );

        }
    }

    player::error player::get_last_error () {
        return this->last_error_;
    }

}
