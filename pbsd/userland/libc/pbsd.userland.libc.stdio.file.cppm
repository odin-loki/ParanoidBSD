module;

#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.file;

/// Minimal FILE concept from hbsd/src/lib/libc/stdio/local.h (logic-only).
export namespace pbsd::userland::libc::stdio {

inline constexpr unsigned kFlagEof = 0x0010U;
inline constexpr unsigned kFlagErr = 0x0020U;

struct IoFile {
    int fd{-1};
    unsigned flags{0};
    unsigned char* r{nullptr};
    unsigned char* w{nullptr};
    unsigned char* bf{nullptr};
    int bfsize{0};
};

[[nodiscard]] inline bool is_eof(const IoFile& fp) noexcept {
    return (fp.flags & kFlagEof) != 0;
}

[[nodiscard]] inline bool is_err(const IoFile& fp) noexcept {
    return (fp.flags & kFlagErr) != 0;
}

inline void set_eof(IoFile& fp) noexcept { fp.flags |= kFlagEof; }
inline void set_err(IoFile& fp) noexcept { fp.flags |= kFlagErr; }
inline void clear_flags(IoFile& fp) noexcept { fp.flags &= ~(kFlagEof | kFlagErr); }

} // namespace pbsd::userland::libc::stdio
