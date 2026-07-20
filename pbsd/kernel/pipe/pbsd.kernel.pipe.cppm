module;
#include <cstdint>

export module pbsd.kernel.pipe;

export import pbsd.core;
export import pbsd.kernel.syscall;

/// Wave 4 — pipe(2)/pipe2(2) flags and sizing (kern/sys_pipe.c).
export namespace pbsd::kernel::pipe {

using namespace pbsd::kernel::syscall;

inline constexpr int kSysPipe2 = 542;

inline constexpr int kOCloexec  = 0x00080000;
inline constexpr int kOClfork   = 0x00100000;
inline constexpr int kONonblock = 0x00000004;

inline constexpr int kAllowedPipe2Flags = kOCloexec | kOClfork | kONonblock;

inline constexpr int kFread     = 0x0001;
inline constexpr int kFwrite    = 0x0002;
inline constexpr int kFnonblock = 0x0004;

inline constexpr std::uint32_t kDefaultPipeBuf = 16384;
inline constexpr std::uint32_t kMinPipeBuf     = 4096;
inline constexpr std::uint32_t kMaxPipeBuf     = 1048576;

struct PipePair {
    int read_fd{-1};
    int write_fd{-1};
};

[[nodiscard]] constexpr Status validate_pipe2_flags(int flags) noexcept {
    if ((flags & ~kAllowedPipe2Flags) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr int pipe_fflags(int o_flags) noexcept {
    int fflags = kFread | kFwrite;
    if ((o_flags & kONonblock) != 0) {
        fflags |= kFnonblock;
    }
    return fflags;
}

[[nodiscard]] constexpr Status validate_pipe_pair(const PipePair& p) noexcept {
    if (p.read_fd < 0 || p.write_fd < 0 || p.read_fd == p.write_fd) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr std::uint32_t clamp_pipe_buf(std::uint32_t req) noexcept {
    if (req < kMinPipeBuf) {
        return kMinPipeBuf;
    }
    if (req > kMaxPipeBuf) {
        return kMaxPipeBuf;
    }
    return req;
}

[[nodiscard]] constexpr bool is_pipe_syscall(int n) noexcept {
    return n == syscall::kSysPipe || n == kSysPipe2;
}

} // namespace pbsd::kernel::pipe
