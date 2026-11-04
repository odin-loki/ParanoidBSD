module;
#include <cstddef>

export module pbsd.stand.smbfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/libsa/smbfs.c
export namespace pbsd::stand::smbfs {

inline constexpr unsigned kMaxPath = 260;
inline constexpr unsigned kShareLen = 80;

[[nodiscard]] inline Status validate_share_name(const char* name) noexcept {
    if (name == nullptr || name[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::stand::smbfs
