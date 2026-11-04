module;
#include <cstdint>

export module pbsd.fs.nullfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/nullfs/null.h — nullfs mount flags.
export namespace pbsd::fs::nullfs {

enum class MountFlag : unsigned long long {
    Cache = 0x0001,
    NoUnpbypass = 0x0002,
};

inline constexpr unsigned kMaxTargetLen = 1024;

[[nodiscard]] inline Status validate_flags(unsigned long long flags) noexcept {
    unsigned long long allowed = static_cast<unsigned long long>(MountFlag::Cache)
        | static_cast<unsigned long long>(MountFlag::NoUnpbypass);
    if ((flags & ~allowed) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_target(char const* target) noexcept {
    if (target == nullptr || target[0] == '\0') {
        return Status::Invalid;
    }
    unsigned len = 0;
    while (target[len] != '\0') {
        if (++len >= kMaxTargetLen) {
            return Status::Invalid;
        }
    }
    return Status::Ok;
}

[[nodiscard]] inline bool cache_enabled(unsigned long long flags) noexcept {
    return (flags & static_cast<unsigned long long>(MountFlag::Cache)) != 0;
}

} // namespace pbsd::fs::nullfs
