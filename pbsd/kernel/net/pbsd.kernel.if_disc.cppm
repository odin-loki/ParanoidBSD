module;
#include <cstdint>

export module pbsd.kernel.if_disc;

import pbsd.core;

/// Freestanding port of `net/if_disc.c` discard pseudo-interface.
export namespace pbsd::kernel::if_disc {

inline constexpr unsigned kDefaultMtu = 65532;
inline constexpr unsigned kTinyMtu    = 1536;
inline constexpr const char kName[]   = "disc";

enum class Ioctl : unsigned long {
    SetFlags = 0x80206910,
    GetFlags = 0x40086911,
};

struct Softc {
    bool attached{};
    unsigned mtu{kDefaultMtu};
};

[[nodiscard]] inline Status attach(Softc& sc, unsigned mtu) noexcept {
    if (sc.attached) {
        return Status::Busy;
    }
    if (mtu < 68 || mtu > kDefaultMtu) {
        return Status::Invalid;
    }
    sc.mtu = mtu;
    sc.attached = true;
    return Status::Ok;
}

[[nodiscard]] inline Status detach(Softc& sc) noexcept {
    if (!sc.attached) {
        return Status::Invalid;
    }
    sc.attached = false;
    return Status::Ok;
}

[[nodiscard]] inline Status output(Softc& sc, unsigned len) noexcept {
    if (!sc.attached) {
        return Status::Invalid;
    }
    if (len > sc.mtu) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::if_disc
