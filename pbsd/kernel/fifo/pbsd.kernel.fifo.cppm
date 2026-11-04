module;
#include <cstdint>

export module pbsd.kernel.fifo;

export import pbsd.core;
export import pbsd.kernel.vnode;

/// Wave 4 — FIFO/mkfifo kernel-side (vfs_syscalls.c, fifofs).
export namespace pbsd::kernel::fifo {

using namespace pbsd::kernel::vnode;

inline constexpr int kSysMkfifo   = 132;
inline constexpr int kSysMkfifoat = 553;

inline constexpr std::uint32_t kSIfifo = 0x1000;
inline constexpr std::uint32_t kSImask = 0xF000;
inline constexpr std::uint32_t kModeMask = 0x0FFF;

inline constexpr int kDtypeFifo = 6;

struct MkfifoRequest {
    const char*   path{};
    std::uint32_t mode{};
};

[[nodiscard]] constexpr bool is_fifo_type(Vtype t) noexcept {
    return t == Vtype::VFifo;
}

[[nodiscard]] constexpr bool is_fifo_mode(std::uint32_t mode) noexcept {
    return (mode & kSImask) == kSIfifo;
}

[[nodiscard]] constexpr std::uint32_t normalize_mode(std::uint32_t mode) noexcept {
    return (mode & kSIfifo) | (mode & kModeMask);
}

[[nodiscard]] constexpr Status validate_mkfifo(const MkfifoRequest& req) noexcept {
    if (req.path == nullptr) {
        return Status::Invalid;
    }
    if (!is_fifo_mode(req.mode)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr bool is_mkfifo_syscall(int n) noexcept {
    return n == kSysMkfifo || n == kSysMkfifoat;
}

[[nodiscard]] constexpr bool fifo_needs_vnode_lock(Vtype t) noexcept {
    return t == Vtype::VFifo;
}

} // namespace pbsd::kernel::fifo
