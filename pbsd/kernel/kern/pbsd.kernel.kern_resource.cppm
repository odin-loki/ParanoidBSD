module;
#include <cstdint>

export module pbsd.kernel.kern_resource;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/kern/kern_resource.c — rlimit scaffold.
export namespace pbsd::kernel::kern_resource {

enum class Limit : unsigned char {
    Cpu = 0,
    Fsize = 1,
    Data = 2,
    Stack = 3,
    Core = 4,
    Rss = 5,
    Nproc = 6,
    Nofile = 7,
    Memlock = 8,
    As = 9,
};

struct Rlimit {
    std::uint64_t cur{};
    std::uint64_t max{};
};

[[nodiscard]] inline Status validate(const Rlimit& r) noexcept {
    if (r.cur > r.max) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status set_cur(Rlimit& r, std::uint64_t cur) noexcept {
    if (cur > r.max) {
        return Status::Denied;
    }
    r.cur = cur;
    return Status::Ok;
}

} // namespace pbsd::kernel::kern_resource
