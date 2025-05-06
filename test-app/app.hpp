#pragma once

#include <cstddef>
#include <functional>

namespace eweb{namespace player{

    /*
        Example:

        initialize();
        on_fragment_video ([](vfragment) { ... });

        on_fragment_audio ([](afragment) { ... });
        // TODO ???
        on_fragment_sync ([](vfragment, afragment) { ... });

    */

    struct fragment_t {
        void *data;
        size_t len;
    };

    using frame_clbck_t = std::function<void (fragment_t fragment)>;

    using sync_clbck_t = std::function<void (fragment_t vfragment, fragment_t afragment)>;

    void initialize () noexcept;

    void push (void *data, size_t len) noexcept;

    void on_fragment_video (frame_clbck_t const& clbck) noexcept;

    void on_fragment_audio (frame_clbck_t const& clbck) noexcept;

    void on_fragment_sync (frame_clbck_t const& clbck) noexcept;

}}
