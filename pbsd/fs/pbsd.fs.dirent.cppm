module;
#include <cstdint>

export module pbsd.fs.dirent;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/sys/dirent.h — DT_* dirent type constants.
export namespace pbsd::fs::dirent {

inline constexpr unsigned kMaxNameLen = 255;

enum class Type : unsigned char {
    Unknown = 0,
    Fifo    = 1,
    Chr     = 2,
    Dir     = 4,
    Blk     = 6,
    Reg     = 8,
    Lnk     = 10,
    Sock    = 12,
    Wht     = 14,
};

struct Entry {
    unsigned long long ino{};
    unsigned char      type{};
    unsigned short     reclen{};
    unsigned char      namelen{};
};

[[nodiscard]] inline constexpr bool is_dir(Type t) noexcept {
    return t == Type::Dir;
}

[[nodiscard]] inline Status validate_entry(Entry const& e) noexcept {
    if (e.namelen > kMaxNameLen) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::dirent
