module;
#include <cstdint>

export module pbsd.kernel.taskqueue;

export import pbsd.core;

/// Wave 5 — taskqueue flags from sys/taskqueue.h.
export namespace pbsd::kernel::taskqueue {

inline constexpr unsigned kNameLen = 32;
inline constexpr unsigned kNumCallbacks = 2;

inline constexpr unsigned kFailIfPending   = 1u << 0;
inline constexpr unsigned kFailIfCanceling = 1u << 1;

enum class CallbackType : unsigned char {
    Init = 0,
    Shutdown = 1,
};

struct TaskqueueStub {
    unsigned flags{};
    bool     pending{};
    bool     canceling{};
};

[[nodiscard]] constexpr Status validate_name_len(unsigned len) noexcept {
    if (len == 0 || len > kNameLen) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr bool fail_if_pending(unsigned flags) noexcept {
    return (flags & kFailIfPending) != 0;
}

[[nodiscard]] inline Status enqueue(TaskqueueStub& tq) noexcept {
    if (fail_if_pending(tq.flags) && tq.pending) {
        return Status::Denied;
    }
    if ((tq.flags & kFailIfCanceling) != 0 && tq.canceling) {
        return Status::Denied;
    }
    tq.pending = true;
    return Status::Ok;
}

} // namespace pbsd::kernel::taskqueue
