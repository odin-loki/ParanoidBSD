module;

#include <cstdint>

export module pbsd.userland.libthr.signals;

import pbsd.core;

/// pthread_sigmask from hbsd/src/lib/libthr/thread/thr_sig.c (signal mask stub)
export namespace pbsd::userland::libthr {

using SigSet = std::uint64_t;

inline SigSet g_thread_sigmask{0};

enum class SigMaskOp : unsigned char { Block, Unblock, SetMask };

[[nodiscard]] inline Status sigmask(SigMaskOp how, SigSet set, SigSet* old = nullptr) noexcept {
    if (old != nullptr) {
        *old = g_thread_sigmask;
    }
    switch (how) {
    case SigMaskOp::Block:
        g_thread_sigmask |= set;
        break;
    case SigMaskOp::Unblock:
        g_thread_sigmask &= ~set;
        break;
    case SigMaskOp::SetMask:
        g_thread_sigmask = set;
        break;
    }
    return Status::Ok;
}

[[nodiscard]] inline SigSet sigmask_current() noexcept { return g_thread_sigmask; }

} // namespace pbsd::userland::libthr
