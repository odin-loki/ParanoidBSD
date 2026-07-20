module;
#include <cstdint>

export module pbsd.kernel.kqueue;

import pbsd.core;
import pbsd.kernel.kevent;

/// Freestanding kqueue(2)/kqueuex(2) glue from sys/event.h (kernel side).
export namespace pbsd::kernel::kqueue {

inline constexpr unsigned kKqueueCloexec  = 0x00000001u;
inline constexpr unsigned kKqueueCponfork = 0x00000002u;

inline constexpr unsigned kKnfListlocked = 0x0001u;
inline constexpr unsigned kKnfNokqlock   = 0x0002u;

inline constexpr unsigned kNoteSignal    = 0x08000000u;

enum class KqueueFlag : unsigned {
    None     = 0,
    Cloexec  = kKqueueCloexec,
    Cponfork = kKqueueCponfork,
};

struct KqueueStub {
    unsigned fd_index{};
    unsigned flags{};
    unsigned registered{};
    unsigned pending{};
    bool     draining{false};
};

struct KnoteStub {
    std::uintptr_t ident{};
    short          filter{};
    unsigned short flags{};
    unsigned       fflags{};
    void*          kn_kq{nullptr};
};

[[nodiscard]] constexpr unsigned kqueuex_flags(unsigned user_flags) noexcept {
    return user_flags & (kKqueueCloexec | kKqueueCponfork);
}

[[nodiscard]] inline Status register_kevent(KqueueStub& kq,
                                            const kevent::KeventStub& ev) noexcept {
    if (!kevent::validate_kevent(ev)) {
        return Status::Invalid;
    }
    ++kq.registered;
    return Status::Ok;
}

[[nodiscard]] inline Status unregister_kevent(KqueueStub& kq,
                                            const kevent::KeventStub& ev) noexcept {
    if (!kevent::validate_kevent(ev)) {
        return Status::Invalid;
    }
    if (kq.registered == 0) {
        return Status::NotFound;
    }
    --kq.registered;
    return Status::Ok;
}

[[nodiscard]] inline Status apply_changelist(KqueueStub& kq,
                                             const kevent::KeventStub* changes,
                                             unsigned nchanges) noexcept {
    if (changes == nullptr && nchanges != 0) {
        return Status::Invalid;
    }
    for (unsigned i = 0; i < nchanges; ++i) {
        const auto& ev = changes[i];
        if (kevent::action_is_delete(ev.flags)) {
            if (unregister_kevent(kq, ev) != Status::Ok) {
                return Status::NotFound;
            }
        } else if (kevent::action_is_add(ev.flags)) {
            if (register_kevent(kq, ev) != Status::Ok) {
                return Status::Invalid;
            }
        } else {
            return Status::Invalid;
        }
    }
    return Status::Ok;
}

[[nodiscard]] inline Status drain_begin(KqueueStub& kq) noexcept {
    if (kq.draining) {
        return Status::Busy;
    }
    kq.draining = true;
    return Status::Ok;
}

[[nodiscard]] inline Status drain_end(KqueueStub& kq) noexcept {
    kq.draining = false;
    kq.pending = 0;
    return Status::Ok;
}

} // namespace pbsd::kernel::kqueue
