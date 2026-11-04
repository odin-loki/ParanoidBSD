module;
#include <cstdint>

export module pbsd.zfs.vdev_queue;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/vdev_queue.h — vdev I/O queue classes.
export namespace pbsd::zfs::vdev_queue {

enum class Priority : unsigned char {
    SyncRead = 0,
    SyncWrite = 1,
    AsyncRead = 2,
    AsyncWrite = 3,
    Scrub = 4,
};

struct QueueStats {
    unsigned pending{};
    unsigned active{};
    unsigned max_active{64};
};

[[nodiscard]] inline Status admit(QueueStats& q) noexcept {
    if (q.active >= q.max_active) {
        return Status::Busy;
    }
    ++q.pending;
    return Status::Ok;
}

[[nodiscard]] inline Status dispatch(QueueStats& q) noexcept {
    if (q.pending == 0) {
        return Status::Invalid;
    }
    --q.pending;
    ++q.active;
    return Status::Ok;
}

} // namespace pbsd::zfs::vdev_queue
