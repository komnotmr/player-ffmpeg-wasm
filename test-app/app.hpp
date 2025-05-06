#pragma once

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libavutil/imgutils.h>
    #include <libavutil/avutil.h>
    #include <libswscale/swscale.h>
}

#include <cstdint>
#include <string>
#include <map>

namespace app{

    class stream_ctx {
        public:
            AVStream *stream;
            AVCodec *codec;
            AVCodecContext *codec_context;
            AVCodecParameters *codec_params;
            int type;
            bool initialized;

            stream_ctx (int type, std::string const& name) :
                type(type),
                initialized(false),
                name_(name)
                { }

            virtual ~stream_ctx ();

            virtual void dump_info () = 0;

        protected:
            std::string name_;
    };

    class video_stream_ctx final : public stream_ctx {
        public:
            video_stream_ctx () :
                stream_ctx(AVMEDIA_TYPE_VIDEO, "video")
                { }

        virtual void dump_info () override;
    };

    class audio_stream_ctx final : public stream_ctx {
        public:
            audio_stream_ctx () :
                stream_ctx(AVMEDIA_TYPE_AUDIO, "audio")
                { }

        virtual void dump_info () override;
    };

    class player {
        public:
            struct error {
                std::string msg;
                uint32_t code;
                error () = default;
                error (std::string const& msg);
                error (uint32_t code, std::string const& msg);
            };

            player ();
            ~player ();

            bool from_source (std::string const& source);
            bool initialize_streams ();

            bool start_read_loop ();

            error get_last_error ();

        private:
            AVFormatContext *format_ctx_;
            std::map<int, stream_ctx*> streams_;
            audio_stream_ctx audio_stream_ctx_;
            video_stream_ctx video_stream_ctx_;
            std::string source_;
            error last_error_;

            bool fail_ (std::string const& msg);
    };

}
