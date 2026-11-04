module;
#include <cstdint>

export module pbsd.fs.specfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/sys/mount.h, kern/vfs_subr.c — special device nodes.
export namespace pbsd::fs::specfs {

enum class DevType : unsigned char {
    Char  = 2,
    Block = 6,
};

struct SpecInfo {
    unsigned major{};
    unsigned minor{};
    DevType type{DevType::Char};
};

[[nodiscard]] inline unsigned dev_to_minor(unsigned dev) noexcept {
    return dev & 0xff;
}

[[nodiscard]] inline unsigned dev_to_major(unsigned dev) noexcept {
    return (dev >> 8) & 0xff;
}

[[nodiscard]] inline unsigned make_dev(unsigned major, unsigned minor) noexcept {
    return ((major & 0xff) << 8) | (minor & 0xff);
}

[[nodiscard]] inline Status validate_dev(DevType type, unsigned major, unsigned minor) noexcept {
    if (type != DevType::Char && type != DevType::Block) {
        return Status::Invalid;
    }
    if (major > 255 || minor > 255) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_spec(SpecInfo const& spec) noexcept {
    return validate_dev(spec.type, spec.major, spec.minor);
}

} // namespace pbsd::fs::specfs
