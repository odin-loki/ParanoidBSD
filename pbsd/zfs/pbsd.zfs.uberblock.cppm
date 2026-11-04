module;
#include <cstdint>

export module pbsd.zfs.uberblock;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/uberblock_impl.h — uberblock magic.
export namespace pbsd::zfs::uberblock {

inline constexpr std::uint64_t kMagic = 0x00bab10cULL;
inline constexpr unsigned kShift = 10;

struct Header {
    std::uint64_t magic{kMagic};
    std::uint64_t txg{};
    std::uint64_t guid{};
};

[[nodiscard]] inline Status validate_magic(std::uint64_t magic) noexcept {
    return magic == kMagic ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline Status validate_header(const Header& h) noexcept {
    if (validate_magic(h.magic) != Status::Ok || h.txg == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::uberblock
