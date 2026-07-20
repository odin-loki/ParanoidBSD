module;
#include <cstdint>

export module pbsd.net.bpf;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/bpf.h — BIOC* ioctls and BPF alignment.
export namespace pbsd::net::bpf {

inline constexpr unsigned kRelease = 199606;
inline constexpr unsigned kAlignment = sizeof(long);

enum class Ioctl : unsigned int {
    Gblen       = 102,
    Setf        = 103,
    Flush       = 104,
    Promisc     = 105,
    Gdlt        = 106,
    Getif       = 107,
    Setif       = 108,
    Version     = 113,
    Immediate   = 112,
    Direction   = 118,
};

[[nodiscard]] inline constexpr unsigned wordalign(unsigned x) noexcept {
    return (x + (kAlignment - 1)) & ~(kAlignment - 1);
}

[[nodiscard]] inline Status validate_ioctl(Ioctl cmd) noexcept {
    if (static_cast<unsigned>(cmd) > static_cast<unsigned>(Ioctl::Direction)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::bpf
