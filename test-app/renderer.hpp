#pragma once

#include <cstddef>

namespace eweb{namespace renderer{

    bool initialize () noexcept;

    void deinitialize () noexcept;

    bool render (void *bytes, size_t len) noexcept;

    bool loop () noexcept;

}}
