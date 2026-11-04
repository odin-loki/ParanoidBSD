module;

#if defined(__FreeBSD__)
#include <sys/capsicum.h>
#endif

export module pbsd.userland.capsicum;

export import pbsd.handles;
export import pbsd.userland.capsicum.os;
export import pbsd.userland.capsicum.rights;
import pbsd.core;

/// Port of hbsd/src/lib/libcapsicum/capsicum_helpers.h core syscall surface.
export namespace pbsd::userland {

/// Capsicum-first sandbox state — no ambient authority after enter().
struct Sandbox {
    bool entered{false};

    [[nodiscard]] Status enter() noexcept {
        if (entered) {
            return Status::Busy;
        }
        const auto st = capsicum::os::cap_enter();
        if (!ok(st.status)) {
            return st.status;
        }
        entered = true;
        return Status::Ok;
    }

    [[nodiscard]] bool active() const noexcept { return entered; }
};

[[nodiscard]] inline CapabilityRights from_cap_bits(unsigned long long bits) noexcept {
    CapabilityRights r = CapabilityRights::None;
    if (bits & 0x1) {
        r = r | CapabilityRights::Read;
    }
    if (bits & 0x2) {
        r = r | CapabilityRights::Write;
    }
    if (bits & 0x4) {
        r = r | CapabilityRights::Grant;
    }
    if (bits & 0x8) {
        r = r | CapabilityRights::Map;
    }
    return r;
}

[[nodiscard]] inline unsigned long long to_cap_bits(CapabilityRights r) noexcept {
    unsigned long long bits = 0;
    if (has_right(r, CapabilityRights::Read)) {
        bits |= 0x1;
    }
    if (has_right(r, CapabilityRights::Write)) {
        bits |= 0x2;
    }
    if (has_right(r, CapabilityRights::Grant)) {
        bits |= 0x4;
    }
    if (has_right(r, CapabilityRights::Map)) {
        bits |= 0x8;
    }
    return bits;
}

namespace capsicum {

/// caph_rights_limit — narrow fd to explicit rights.
[[nodiscard]] inline StatusOnly rights_limit(int fd, CapabilityRights rights) noexcept {
    auto cap = os::CapRights::init_base();
#if defined(__FreeBSD__)
    if (has_right(rights, CapabilityRights::Read)) {
        cap.add(CAP_READ);
    }
    if (has_right(rights, CapabilityRights::Write)) {
        cap.add(CAP_WRITE);
    }
    if (has_right(rights, CapabilityRights::Map)) {
        cap.add(CAP_LOOKUP);
    }
#else
    (void)rights;
#endif
    return os::rights_limit(fd, cap);
}

/// caph_ioctls_limit
[[nodiscard]] inline StatusOnly ioctls_limit(int fd) noexcept { return os::ioctls_limit(fd); }

/// caph_fcntls_limit
[[nodiscard]] inline StatusOnly fcntls_limit(int fd) noexcept { return os::fcntls_limit(fd); }

/// caph_enter
[[nodiscard]] inline StatusOnly enter() noexcept { return os::cap_enter(); }

/// caph_enter_casper — CASPER_SUPPORT gate preserved as compile-time stub on non-BSD.
[[nodiscard]] inline StatusOnly enter_casper() noexcept {
#if defined(CASPER_SUPPORT) && CASPER_SUPPORT != 0
    return enter();
#else
    return enter();
#endif
}

inline void cache_tzdata() noexcept { os::cache_tzdata(); }
inline void cache_catpages() noexcept { os::cache_catpages(); }

} // namespace capsicum

} // namespace pbsd::userland
