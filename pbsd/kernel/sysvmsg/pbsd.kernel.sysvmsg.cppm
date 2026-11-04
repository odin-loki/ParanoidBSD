module;
#include <cstdint>

export module pbsd.kernel.sysvmsg;

export import pbsd.core;

/// Wave 4/5 — SysV message queues from sys/msg.h, kern/kern_msg.c.
export namespace pbsd::kernel::sysvmsg {

inline constexpr int kMsgR = 0400;
inline constexpr int kMsgW = 0200;

inline constexpr int kIpcCreat  = 00001000;
inline constexpr int kIpcExcl   = 00002000;
inline constexpr int kIpcNowait = 00004000;
inline constexpr int kIpcRm     = 0;
inline constexpr int kIpcSet    = 1;
inline constexpr int kIpcStat   = 2;
inline constexpr int kIpcInfo   = 3;

inline constexpr int kMsgNoerror = 0x4000;
inline constexpr int kMsgExcl    = 0x8000;

inline constexpr int kMsgmax = 0;
inline constexpr int kMsgmnb = 1;
inline constexpr int kMsgmni = 2;
inline constexpr int kMsgssz = 3;
inline constexpr int kMsgtql = 4;

struct MsqidDs {
    std::uint64_t msg_cbytes{};
    std::uint64_t msg_qnum{};
    std::uint64_t msg_qbytes{};
    std::uint32_t msg_lspid{};
    std::uint32_t msg_lrpid{};
    std::uint32_t msg_stime{};
    std::uint32_t msg_rtime{};
    std::uint32_t msg_ctime{};
    std::uint16_t msg_perm_mode{};
    int           msg_perm_key{};
};

[[nodiscard]] constexpr Status validate_msg_size(std::uint64_t size) noexcept {
    if (size == 0 || size > 65536) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr bool msg_nowait(int flags) noexcept {
    return (flags & kIpcNowait) != 0;
}

[[nodiscard]] constexpr bool msg_noerror(int flags) noexcept {
    return (flags & kMsgNoerror) != 0;
}

} // namespace pbsd::kernel::sysvmsg
