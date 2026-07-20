module;
#include <cstddef>

export module pbsd.stand.pkgfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/pkgfs.c
export namespace pbsd::stand::pkgfs {

inline constexpr unsigned kMaxPackages = 32;
inline constexpr unsigned kNameLen = 64;

enum class MountState : unsigned char {
    Unmounted = 0,
    Mounted = 1,
};

[[nodiscard]] inline Status validate_name(const char* name) noexcept {
    if (name == nullptr || name[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::stand::pkgfs
