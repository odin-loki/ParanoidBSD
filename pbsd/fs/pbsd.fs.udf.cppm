module;
#include <cstdint>

export module pbsd.fs.udf;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/fs/udf/ecma167-udf.h, udf.h — UDF on-disk constants.
export namespace pbsd::fs::udf {

inline constexpr unsigned kHashtblSize = 100;
inline constexpr unsigned kRegidIdSize = 23;
inline constexpr unsigned kFidSize = 38;
inline constexpr unsigned kFentrySize = 176;

enum class FileChar : unsigned {
    Visible = 1 << 0,
    Dir     = 1 << 1,
    Deleted = 1 << 2,
    Parent  = 1 << 3,
    Meta    = 1 << 4,
};

enum class PathType : unsigned char {
    Root   = 2,
    DotDot = 3,
    Dot    = 4,
    Path   = 5,
};

struct MountArgs {
    unsigned part_start{};
    unsigned part_len{};
    unsigned bsize{};
    unsigned long long root_id{};
};

[[nodiscard]] inline bool file_is_dir(unsigned char chars) noexcept {
    return (chars & static_cast<unsigned>(FileChar::Dir)) != 0;
}

[[nodiscard]] inline bool file_is_deleted(unsigned char chars) noexcept {
    return (chars & static_cast<unsigned>(FileChar::Deleted)) != 0;
}

[[nodiscard]] inline Status validate_mount(MountArgs const& args) noexcept {
    if (args.bsize == 0 || args.part_len == 0) {
        return Status::Invalid;
    }
    if ((args.bsize & (args.bsize - 1)) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_path_type(PathType type) noexcept {
    switch (type) {
    case PathType::Root:
    case PathType::DotDot:
    case PathType::Dot:
    case PathType::Path:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::fs::udf
