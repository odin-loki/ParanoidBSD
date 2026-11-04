module;
#include <cstdint>

export module pbsd.kernel.subr_msgbuf;

import pbsd.core;

/// Freestanding port of `subr_msgbuf.c` — kernel message buffer metadata.
export namespace pbsd::kernel::subr_msgbuf {

inline constexpr unsigned kDefaultSize = 4096;

struct Msgbuf {
    unsigned size{kDefaultSize};
    unsigned head{};
    unsigned tail{};
    unsigned seq{};
    bool enabled{true};
};

[[nodiscard]] inline Status init(Msgbuf& mb, unsigned size) noexcept {
    if (size < 256) {
        return Status::Invalid;
    }
    mb.size = size;
    mb.head = 0;
    mb.tail = 0;
    mb.seq = 0;
    mb.enabled = true;
    return Status::Ok;
}

[[nodiscard]] inline unsigned available(Msgbuf const& mb) noexcept {
    if (!mb.enabled || mb.size <= mb.head) {
        return 0;
    }
    return mb.size - mb.head;
}

[[nodiscard]] inline Status append(Msgbuf& mb, unsigned len) noexcept {
    if (!mb.enabled) {
        return Status::Invalid;
    }
    if (mb.head + len > mb.size) {
        return Status::Denied;
    }
    mb.head += len;
    ++mb.seq;
    return Status::Ok;
}

inline void clear(Msgbuf& mb) noexcept {
    mb.head = 0;
    mb.tail = 0;
}

} // namespace pbsd::kernel::subr_msgbuf
