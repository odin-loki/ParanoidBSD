module;
#include <cstdint>

export module pbsd.stand.zfsboot;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/common/zfs_cmd.c — ZFS boot environment flags.
export namespace pbsd::stand::zfsboot {

enum class BootFlag : unsigned int {
    Default = 0x00000001,
    Active  = 0x00000002,
    Fail    = 0x00000004,
};

struct BootEnv {
    char name[64]{};
    unsigned flags{};
};

[[nodiscard]] inline Status validate_name(BootEnv const& be) noexcept {
    if (be.name[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline constexpr bool flag_has(unsigned f, BootFlag bit) noexcept {
    return (f & static_cast<unsigned>(bit)) != 0;
}

[[nodiscard]] inline Status validate_flags(unsigned f) noexcept {
    if (f & static_cast<unsigned>(BootFlag::Fail)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::stand::zfsboot
