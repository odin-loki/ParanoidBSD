module;
#include <cstdint>

export module pbsd.geom.linux_lvm;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/linux_lvm/g_linux_lvm.h — Linux LVM label constants.
export namespace pbsd::geom::linux_lvm {

inline constexpr unsigned kNameLen = 128;
inline constexpr unsigned kUuidLen = 40;

struct Label {
    std::uint64_t sector{};
    std::uint32_t crc{};
    char uuid[kUuidLen]{};
    std::uint64_t size{};
};

[[nodiscard]] inline Status validate_label(const Label& lbl) noexcept {
    if (lbl.sector == 0 || lbl.size == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::geom::linux_lvm
