module;
#include <cstdint>

export module pbsd.fs.unionfs;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/unionfs/union.h — unionfs copy/whiteout modes.
export namespace pbsd::fs::unionfs {

enum class CopyMode : unsigned char {
    Traditional = 0,
    Transparent = 1,
    Masquerade  = 2,
};

enum class WhiteMode : unsigned char {
    Always = 0,
    WhenNeeded = 1,
};

[[nodiscard]] inline Status validate_copy_mode(CopyMode m) noexcept {
    switch (m) {
    case CopyMode::Traditional:
    case CopyMode::Transparent:
    case CopyMode::Masquerade:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::fs::unionfs
