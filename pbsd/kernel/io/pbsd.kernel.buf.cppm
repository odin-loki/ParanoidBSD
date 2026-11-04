module;
#include <cstdint>

export module pbsd.kernel.buf;

export import pbsd.core;

/// Wave 4/5 — buffer flags from sys/buf.h.
export namespace pbsd::kernel::buf {

inline constexpr std::uint32_t kAge           = 0x00000001;
inline constexpr std::uint32_t kNeedCommit    = 0x00000002;
inline constexpr std::uint32_t kAsync         = 0x00000004;
inline constexpr std::uint32_t kDirect       = 0x00000008;
inline constexpr std::uint32_t kDeferred      = 0x00000010;
inline constexpr std::uint32_t kCache         = 0x00000020;
inline constexpr std::uint32_t kValidSuspWrt  = 0x00000040;
inline constexpr std::uint32_t kInval         = 0x00002000;
inline constexpr std::uint32_t kLocked        = 0x00004000;
inline constexpr std::uint32_t kBusy          = 0x00008000;
inline constexpr std::uint32_t kDone          = 0x00010000;
inline constexpr std::uint32_t kDelWri        = 0x00020000;
inline constexpr std::uint32_t kClusterOk     = 0x00200000;
inline constexpr std::uint32_t kPaging        = 0x00400000;
inline constexpr std::uint32_t kRelBuf        = 0x00800000;
inline constexpr std::uint32_t kVmio          = 0x01000000;

struct BufStub {
    std::uint32_t flags{};
    int           error{};
};

[[nodiscard]] constexpr bool is_async(std::uint32_t flags) noexcept {
    return (flags & kAsync) != 0;
}

[[nodiscard]] constexpr bool is_dirty(std::uint32_t flags) noexcept {
    return (flags & kDelWri) != 0;
}

[[nodiscard]] constexpr Status validate_flags(std::uint32_t flags) noexcept {
    if ((flags & kDone) != 0 && (flags & kBusy) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::buf
