module;

export module pbsd.userland.libthr.cancel;

import pbsd.core;
import pbsd.userland.libthr.mutex;
import pbsd.userland.libthr.thread;

/// pthread_cancel/setcancelstate from hbsd/src/lib/libthr/thread/thr_cancel.c
export namespace pbsd::userland::libthr {

enum class CancelState : unsigned char { Enable, Disable };
enum class CancelType : unsigned char { Deferred, Asynchronous };

inline CancelState g_cancel_state{CancelState::Enable};
inline CancelType g_cancel_type{CancelType::Deferred};
inline bool g_cancel_pending{false};

[[nodiscard]] inline Status thread_cancel(ThreadId tid) noexcept {
    if (tid == kInvalidThread) {
        return Status::Invalid;
    }
    g_cancel_pending = true;
    return Status::Ok;
}

[[nodiscard]] inline Status setcancelstate(CancelState state, CancelState* old = nullptr) noexcept {
    if (old != nullptr) {
        *old = g_cancel_state;
    }
    g_cancel_state = state;
    return Status::Ok;
}

[[nodiscard]] inline Status setcanceltype(CancelType type, CancelType* old = nullptr) noexcept {
    if (old != nullptr) {
        *old = g_cancel_type;
    }
    g_cancel_type = type;
    return Status::Ok;
}

inline void testcancel() noexcept {
    if (g_cancel_state == CancelState::Enable && g_cancel_pending) {
        g_cancel_pending = false;
    }
}

} // namespace pbsd::userland::libthr
