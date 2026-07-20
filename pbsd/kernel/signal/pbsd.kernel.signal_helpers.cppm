module;
#include <cstdint>

export module pbsd.kernel.signal_helpers;

import pbsd.core;
import pbsd.kernel.signal;

/// sigset(3)/sigaction helpers from sys/signal.h (freestanding subset).
export namespace pbsd::kernel::signal_helpers {

using signal::kSigKill;
using signal::kSigStop;

struct Sigset {
    std::uint32_t bits{0};

    [[nodiscard]] constexpr bool empty() const noexcept {
        return bits == 0;
    }
};

[[nodiscard]] constexpr Sigset sigemptyset() noexcept {
    return Sigset{};
}

[[nodiscard]] constexpr Sigset sigfillset() noexcept {
    return Sigset{~0u};
}

[[nodiscard]] inline Status sigaddset(Sigset& set, int sig) noexcept {
    if (sig <= 0 || sig > 31) {
        return Status::Invalid;
    }
    set.bits |= (1u << (static_cast<unsigned>(sig) - 1u));
    return Status::Ok;
}

[[nodiscard]] inline Status sigdelset(Sigset& set, int sig) noexcept {
    if (sig <= 0 || sig > 31) {
        return Status::Invalid;
    }
    set.bits &= ~(1u << (static_cast<unsigned>(sig) - 1u));
    return Status::Ok;
}

[[nodiscard]] inline bool sigismember(const Sigset& set, int sig) noexcept {
    if (sig <= 0 || sig > 31) {
        return false;
    }
    return (set.bits & (1u << (static_cast<unsigned>(sig) - 1u))) != 0;
}

[[nodiscard]] inline bool sigcantmask(int sig) noexcept {
    return sig == kSigKill || sig == kSigStop;
}

[[nodiscard]] inline Status sigprocmask_add(const Sigset& current, const Sigset& add,
                                            Sigset& out) noexcept {
    out.bits = current.bits | add.bits;
    return Status::Ok;
}

[[nodiscard]] inline Status sigprocmask_del(const Sigset& current, const Sigset& del,
                                            Sigset& out) noexcept {
    out.bits = current.bits & ~del.bits;
    return Status::Ok;
}

[[nodiscard]] inline bool sigset_equal(const Sigset& a, const Sigset& b) noexcept {
    return a.bits == b.bits;
}

} // namespace pbsd::kernel::signal_helpers
