module;
#include <cstdint>

export module pbsd.zfs.vdev_label;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/include/sys/vdev_label.h — vdev label count.
export namespace pbsd::zfs::vdev_label {

inline constexpr unsigned kLabels = 4;
inline constexpr unsigned kPadSize = 8192;

struct Config {
    std::uint64_t guid{};
    unsigned children{};
    unsigned ashift{9};
};

[[nodiscard]] inline Status validate_ashift(unsigned ashift) noexcept {
    if (ashift < 9 || ashift > 16) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_config(const Config& c) noexcept {
    if (c.guid == 0) {
        return Status::Invalid;
    }
    return validate_ashift(c.ashift);
}

[[nodiscard]] inline std::uint64_t label_offset(unsigned label_index,
                                                  std::uint64_t psize) noexcept {
    if (label_index >= kLabels) {
        return 0;
    }
    return label_index * (psize / kLabels);
}

} // namespace pbsd::zfs::vdev_label
