module;
#include <cstdint>

export module pbsd.fs.stat;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/sys/stat.h — S_IF* file type bits and UF_* flags.
export namespace pbsd::fs::stat {

inline constexpr unsigned kBlksize = 512;

enum class FileType : unsigned int {
    Fifo    = 0x1000,
    Char    = 0x2000,
    Dir     = 0x4000,
    Block   = 0x6000,
    Regular = 0x8000,
    Link    = 0xA000,
    Socket  = 0xC000,
    Whiteout = 0xE000,
};

enum class UfFlag : unsigned int {
    Nodump   = 0x00000001,
    Immutable = 0x00000002,
    Append   = 0x00000004,
    Opaque   = 0x00000008,
    Nounlink = 0x00000010,
    Hidden   = 0x00008000,
    Archive  = 0x00010000,
};

[[nodiscard]] inline constexpr FileType mode_to_type(unsigned mode) noexcept {
    return static_cast<FileType>(mode & 0xF000);
}

[[nodiscard]] inline constexpr bool is_dir(unsigned mode) noexcept {
    return mode_to_type(mode) == FileType::Dir;
}

[[nodiscard]] inline Status validate_mode(unsigned mode) noexcept {
    switch (mode_to_type(mode)) {
    case FileType::Fifo:
    case FileType::Char:
    case FileType::Dir:
    case FileType::Block:
    case FileType::Regular:
    case FileType::Link:
    case FileType::Socket:
    case FileType::Whiteout:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::fs::stat
