module;
#include <cstdint>

export module pbsd.kernel.select;

export import pbsd.core;
export import pbsd.kernel.syscall;
export import pbsd.kernel.poll;

/// Wave 4 — select(2)/pselect(2) fd_set helpers (sys/select.h, kern/subr_select.c).
export namespace pbsd::kernel::select {

using namespace pbsd::kernel::syscall;
using namespace pbsd::kernel::poll;

inline constexpr int kSysSelect  = 93;
inline constexpr int kSysPselect = 522;

inline constexpr int kFdSetBits  = static_cast<int>(sizeof(unsigned long) * 8);
inline constexpr int kDefaultFdSetSize = 1024;

struct FdSet {
    unsigned long bits[16]{};
};

[[nodiscard]] constexpr bool fd_isset(int fd, const FdSet& set) noexcept {
    if (fd < 0 || fd >= kDefaultFdSetSize) {
        return false;
    }
    const int idx = fd / kFdSetBits;
    const int bit = fd % kFdSetBits;
    return (set.bits[idx] & (1UL << bit)) != 0;
}

[[nodiscard]] constexpr void fd_set(int fd, FdSet& set) noexcept {
    if (fd < 0 || fd >= kDefaultFdSetSize) {
        return;
    }
    const int idx = fd / kFdSetBits;
    const int bit = fd % kFdSetBits;
    set.bits[idx] |= (1UL << bit);
}

[[nodiscard]] constexpr void fd_clr(int fd, FdSet& set) noexcept {
    if (fd < 0 || fd >= kDefaultFdSetSize) {
        return;
    }
    const int idx = fd / kFdSetBits;
    const int bit = fd % kFdSetBits;
    set.bits[idx] &= ~(1UL << bit);
}

[[nodiscard]] constexpr void fd_zero(FdSet& set) noexcept {
    for (auto& w : set.bits) {
        w = 0;
    }
}

[[nodiscard]] constexpr Status validate_nfds(int nfds) noexcept {
    if (nfds < 0 || nfds > kDefaultFdSetSize) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr bool is_select_syscall(int n) noexcept {
    return n == kSysSelect || n == kSysPselect;
}

[[nodiscard]] constexpr Event poll_from_readset(int fd, const FdSet& rset,
                                                const FdSet& wset,
                                                const FdSet& eset) noexcept {
    Event ev = Event::None;
    if (fd_isset(fd, rset)) {
        ev = ev | Event::In;
    }
    if (fd_isset(fd, wset)) {
        ev = ev | Event::Out;
    }
    if (fd_isset(fd, eset)) {
        ev = ev | Event::Err;
    }
    return ev;
}

} // namespace pbsd::kernel::select
