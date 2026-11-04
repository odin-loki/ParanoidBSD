module;
#include <cstdint>

export module pbsd.kernel.uio;

export import pbsd.core;

/// Wave 5 — uio scatter/gather limits from sys/uio.h, sys/_uio.h.
export namespace pbsd::kernel::uio {

inline constexpr unsigned kMaxIov = 1024;

enum class Seg : unsigned char {
    Userspace = 0,
    Sysspace  = 1,
    Nocopy    = 2,
};

enum class Rw : unsigned char {
    Read  = 0,
    Write = 1,
};

struct UioStub {
    int     iovcnt{};
    std::int64_t offset{};
    std::int64_t resid{};
    Seg     seg{Seg::Userspace};
    Rw      rw{Rw::Read};
};

[[nodiscard]] constexpr Status validate_iovcnt(int cnt) noexcept {
    if (cnt <= 0 || cnt > static_cast<int>(kMaxIov)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_resid(std::int64_t resid) noexcept {
    if (resid < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr bool is_read(Rw rw) noexcept {
    return rw == Rw::Read;
}

[[nodiscard]] constexpr bool is_userspace(Seg seg) noexcept {
    return seg == Seg::Userspace;
}

} // namespace pbsd::kernel::uio
