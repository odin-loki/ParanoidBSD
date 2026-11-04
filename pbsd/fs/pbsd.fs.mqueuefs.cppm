module;
#include <cstdint>

export module pbsd.fs.mqueuefs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/mqueuefs — POSIX mqueue vnode scaffold.
export namespace pbsd::fs::mqueuefs {

inline constexpr unsigned kMaxQueues = 256;
inline constexpr unsigned kDefaultMsgMax = 10;
inline constexpr unsigned kDefaultMsgSize = 8192;

struct Queue {
    unsigned maxmsg{kDefaultMsgMax};
    unsigned msgsize{kDefaultMsgSize};
    unsigned depth{};
    bool open{false};
};

[[nodiscard]] inline Status open_queue(Queue& q, unsigned maxmsg, unsigned msgsize) noexcept {
    if (maxmsg == 0 || msgsize == 0 || maxmsg > 1024 || msgsize > 1'048'576) {
        return Status::Invalid;
    }
    q.maxmsg = maxmsg;
    q.msgsize = msgsize;
    q.depth = 0;
    q.open = true;
    return Status::Ok;
}

[[nodiscard]] inline Status enqueue(Queue& q) noexcept {
    if (!q.open) {
        return Status::Invalid;
    }
    if (q.depth >= q.maxmsg) {
        return Status::Busy;
    }
    ++q.depth;
    return Status::Ok;
}

[[nodiscard]] inline Status dequeue(Queue& q) noexcept {
    if (!q.open || q.depth == 0) {
        return Status::NotFound;
    }
    --q.depth;
    return Status::Ok;
}

} // namespace pbsd::fs::mqueuefs
