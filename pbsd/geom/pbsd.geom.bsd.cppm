module;
#include <cstdint>

export module pbsd.geom.bsd;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/bsd/g_bsd.h — BSD disklabel slice constants.
export namespace pbsd::geom::bsd {

inline constexpr unsigned kMaxPartitions = 16;
inline constexpr unsigned kMagic = 0x82564557u;

struct Label {
    std::uint32_t magic{kMagic};
    unsigned short nparts{};
    unsigned long long disk_size{};
};

[[nodiscard]] inline Status validate_magic(std::uint32_t magic) noexcept {
    return magic == kMagic ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline Status validate_label(const Label& lbl) noexcept {
    if (validate_magic(lbl.magic) != Status::Ok) {
        return Status::Invalid;
    }
    if (lbl.nparts == 0 || lbl.nparts > kMaxPartitions) {
        return Status::Invalid;
    }
    if (lbl.disk_size == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::geom::bsd
