#pragma once

#include <cstddef>
#include <functional>

#include <emscripten.h>
#include <emscripten/bind.h>

namespace eweb{namespace player{

    void debug (bool on_off); 

    /**
    *  audio/video data
    */
    struct fragment_t {
        uint8_t *data;
        size_t len;
    };

    using frame_clbck_t = std::function<void (fragment_t fragment)>;

    using sync_clbck_t = std::function<void (fragment_t vfragment, fragment_t afragment)>;

    /**
    * call clbck
    */
    void call_clbck_by_name (std::string const& clbck_name);

    /**
    * initialize library (ffmpeg, etc...)
    */
    void initialize () noexcept;

    /**
    * deinitialize library (ffmpeg, etc...)
    */
    void deinitialize () noexcept;

    /**
    * push media stream chunk to the internal queue
    */
    void push (void *data, size_t len) noexcept;

    /**
    * process data to queue, calls clbacks on video/audio fragments.
    * in must be called inside loop
    */
    void process_queue () noexcept;

    /**
    * seeks by tytes in queue
    */
    bool seek_to_bytes (size_t bytes) noexcept;

    /**
    * sets callback which will be called on decoded video frame
    */
    void on_fragment_video (frame_clbck_t const& clbck) noexcept;

    /**
    * sets callback which will be called on decoded audio frame
    */
    void on_fragment_audio (frame_clbck_t const& clbck) noexcept;

    /**
    * sets callback which will be called on decoded and synced video and audio frames
    */
    void on_fragment_sync (sync_clbck_t const& clbck) noexcept; // TODO

}}
