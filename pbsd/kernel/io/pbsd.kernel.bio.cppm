module;
#include <cstdint>

export module pbsd.kernel.bio;

export import pbsd.core;

/// Wave 4/5 — GEOM bio commands/flags from sys/bio.h.
export namespace pbsd::kernel::bio {

inline constexpr unsigned char kRead    = 0x01;
inline constexpr unsigned char kWrite   = 0x02;
inline constexpr unsigned char kDelete  = 0x03;
inline constexpr unsigned char kGetattr = 0x04;
inline constexpr unsigned char kFlush   = 0x05;
inline constexpr unsigned char kZone    = 0x09;
inline constexpr unsigned char kSpeedup = 0x0a;

inline constexpr unsigned kDone           = 0x02;
inline constexpr unsigned kOnqueue        = 0x04;
inline constexpr unsigned kOrdered        = 0x08;
inline constexpr unsigned kUnmapped       = 0x10;
inline constexpr unsigned kTransientMap   = 0x20;
inline constexpr unsigned kVlist          = 0x40;
inline constexpr unsigned kSwap           = 0x200;
inline constexpr unsigned kError          = 0x1000;
inline constexpr unsigned kExterr         = 0x2000;
inline constexpr unsigned kSpeedupWrite   = 0x4000;
inline constexpr unsigned kSpeedupTrim    = 0x8000;

[[nodiscard]] constexpr bool is_read(unsigned char cmd) noexcept {
    return cmd == kRead;
}

[[nodiscard]] constexpr bool is_write(unsigned char cmd) noexcept {
    return cmd == kWrite || cmd == kDelete;
}

[[nodiscard]] constexpr Status validate_cmd(unsigned char cmd) noexcept {
    if (cmd < kRead || (cmd > kFlush && cmd < kZone) || cmd > kSpeedup) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::bio
