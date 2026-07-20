module;

#include <cstdint>

export module pbsd.userland.libc.gen.getpeereid;

export import pbsd.core;

/// getpeereid scaffold from hbsd/src/lib/libc/gen/getpeereid.c
export namespace pbsd::userland::libc {

struct PeerCred {
    std::uint32_t uid{0};
    std::uint32_t gid{0};
};

[[nodiscard]] inline StatusOnly getpeereid(int /*fd*/, PeerCred& out) noexcept {
    // Hosted scaffold until socket credential syscall is wired.
    out = PeerCred{};
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc
