module;
#include <cstdint>

export module pbsd.net.ifq;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/ifq.h — interface output queue limits.
export namespace pbsd::net::ifq {

inline constexpr int kDunitNone = -1;
inline constexpr unsigned kDefaultMaxLen = 50;

struct Queue {
    unsigned len{};
    unsigned maxlen{kDefaultMaxLen};
};

[[nodiscard]] inline Status validate_maxlen(unsigned maxlen) noexcept {
    if (maxlen == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status enqueue(Queue& q) noexcept {
    if (q.len >= q.maxlen) {
        return Status::Busy;
    }
    ++q.len;
    return Status::Ok;
}

[[nodiscard]] inline Status dequeue(Queue& q) noexcept {
    if (q.len == 0) {
        return Status::Invalid;
    }
    --q.len;
    return Status::Ok;
}

} // namespace pbsd::net::ifq
