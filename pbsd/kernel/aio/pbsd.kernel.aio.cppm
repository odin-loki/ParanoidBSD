module;
#include <cstdint>

export module pbsd.kernel.aio;

export import pbsd.core;
export import pbsd.kernel.syscall;

/// Wave 4 — aio(2)/lio_listio constants (sys/aio.h, kern/kern_aio.c).
export namespace pbsd::kernel::aio {

using namespace pbsd::kernel::syscall;

inline constexpr int kSysAioRead         = 255;
inline constexpr int kSysAioWrite        = 256;
inline constexpr int kSysAioReturn       = 314;
inline constexpr int kSysAioSuspend      = 315;
inline constexpr int kSysAioCancel       = 316;
inline constexpr int kSysAioError        = 317;
inline constexpr int kSysAioWaitcomplete = 359;
inline constexpr int kSysAioFsync        = 465;
inline constexpr int kSysAioMlock        = 543;
inline constexpr int kSysAioWritev       = 578;
inline constexpr int kSysAioReadv        = 579;

inline constexpr int kAioCanceled    = 0x1;
inline constexpr int kAioNotcanceled = 0x2;
inline constexpr int kAioAlldone     = 0x3;

inline constexpr int kLioNop      = 0x0;
inline constexpr int kLioWrite    = 0x1;
inline constexpr int kLioRead     = 0x2;
inline constexpr int kLioVectored = 0x4;
inline constexpr int kLioWritev   = kLioWrite | kLioVectored;
inline constexpr int kLioReadv    = kLioRead | kLioVectored;
inline constexpr int kLioSync     = 0x8;
inline constexpr int kLioDsync    = 0x10 | kLioSync;
inline constexpr int kLioMlock    = 0x20;

inline constexpr int kLioNowait = 0x0;
inline constexpr int kLioWait   = 0x1;

inline constexpr int kAioListioMax = 16;

inline constexpr std::uint32_t kAioOp2Foffset  = 0x00000001;
inline constexpr std::uint32_t kAioOp2Vectored = 0x00000002;

struct AioEntry {
    int         opcode{};
    int         fd{-1};
    std::uint64_t offset{};
    std::uint32_t nbytes{};
};

[[nodiscard]] inline unsigned aio_syscall_table_size() noexcept {
    return 10;
}

[[nodiscard]] constexpr bool is_aio_syscall(int n) noexcept {
    return n == kSysAioRead || n == kSysAioWrite || n == kSysAioReturn
        || n == kSysAioSuspend || n == kSysAioCancel || n == kSysAioError
        || n == kSysAioWaitcomplete || n == kSysAioFsync || n == kSysAioMlock
        || n == kSysAioWritev || n == kSysAioReadv;
}

[[nodiscard]] constexpr Status validate_lio_opcode(int op) noexcept {
    if (op == kLioNop || op == kLioWrite || op == kLioRead
        || op == kLioWritev || op == kLioReadv) {
        return Status::Ok;
    }
    return Status::Invalid;
}

[[nodiscard]] constexpr Status validate_listio_count(unsigned count) noexcept {
    if (count == 0 || count > static_cast<unsigned>(kAioListioMax)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_aio_entry(const AioEntry& e) noexcept {
    if (e.opcode == kLioNop) {
        return Status::Ok;
    }
    if (validate_lio_opcode(e.opcode) != Status::Ok) {
        return Status::Invalid;
    }
    if (e.fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr bool cancel_complete(int result) noexcept {
    return result == kAioCanceled || result == kAioAlldone;
}

} // namespace pbsd::kernel::aio
