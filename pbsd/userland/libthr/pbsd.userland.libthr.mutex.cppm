module;

#include <cstddef>
#include <cstdint>

export module pbsd.userland.libthr.mutex;

import pbsd.core;

/// pthread_mutex concepts from hbsd/src/lib/libthr/thread/thr_mutex.c
export namespace pbsd::userland::libthr {

using ThreadId = std::uint64_t;
inline constexpr ThreadId kInvalidThread = 0;

enum class MutexType : unsigned char { Normal, Recursive, ErrorCheck };

struct Mutex {
    MutexType type{MutexType::Normal};
    ThreadId owner{kInvalidThread};
    unsigned lock_count{0};
};

[[nodiscard]] inline Status mutex_init(Mutex& m, MutexType t = MutexType::Normal) noexcept {
    m = Mutex{};
    m.type = t;
    return Status::Ok;
}

[[nodiscard]] inline Status mutex_lock(Mutex& m, ThreadId self) noexcept {
    if (self == kInvalidThread) {
        return Status::Invalid;
    }
    if (m.owner == kInvalidThread) {
        m.owner = self;
        m.lock_count = 1;
        return Status::Ok;
    }
    if (m.owner == self && m.type == MutexType::Recursive) {
        ++m.lock_count;
        return Status::Ok;
    }
    if (m.owner == self && m.type == MutexType::ErrorCheck) {
        return Status::Busy;
    }
    return Status::Busy;
}

[[nodiscard]] inline Status mutex_unlock(Mutex& m, ThreadId self) noexcept {
    if (m.owner != self) {
        return Status::Denied;
    }
    if (m.lock_count > 1) {
        --m.lock_count;
        return Status::Ok;
    }
    m.owner = kInvalidThread;
    m.lock_count = 0;
    return Status::Ok;
}

[[nodiscard]] inline Status mutex_trylock(Mutex& m, ThreadId self) noexcept {
    if (m.owner != kInvalidThread && m.owner != self) {
        return Status::Busy;
    }
    return mutex_lock(m, self);
}

} // namespace pbsd::userland::libthr
