module;
#include <cstdint>

export module pbsd.kernel.socket_syscall;

export import pbsd.core;
export import pbsd.kernel.syscall;

/// Wave 4 — socket-related syscall numbers and validation (uipc_syscalls.c).
export namespace pbsd::kernel::socket_syscall {

using namespace pbsd::kernel::syscall;

inline constexpr int kSysSocket     = 97;
inline constexpr int kSysConnect    = 98;
inline constexpr int kSysBind       = 104;
inline constexpr int kSysListen     = 106;
inline constexpr int kSysAccept     = 30;
inline constexpr int kSysSendto     = 133;
inline constexpr int kSysRecvfrom   = 29;
inline constexpr int kSysSendmsg    = 28;
inline constexpr int kSysRecvmsg    = 27;
inline constexpr int kSysGetsockopt = 118;
inline constexpr int kSysSetsockopt = 105;
inline constexpr int kSysSocketpair  = 135;
inline constexpr int kSysShutdown   = 134;

inline constexpr int kAfUnspec  = 0;
inline constexpr int kAfLocal   = 1;
inline constexpr int kAfInet    = 2;
inline constexpr int kAfInet6   = 28;

inline constexpr int kSockStream = 1;
inline constexpr int kSockDgram  = 2;
inline constexpr int kSockRaw    = 3;

struct SyscallEntry {
    int         number;
    const char* name;
};

inline constexpr SyscallEntry kSocketSyscallTable[] = {
    {kSysSocket,     "socket"},
    {kSysConnect,    "connect"},
    {kSysBind,       "bind"},
    {kSysListen,     "listen"},
    {kSysAccept,     "accept"},
    {kSysSendto,     "sendto"},
    {kSysRecvfrom,   "recvfrom"},
    {kSysSendmsg,    "sendmsg"},
    {kSysRecvmsg,    "recvmsg"},
    {kSysGetsockopt, "getsockopt"},
    {kSysSetsockopt, "setsockopt"},
    {kSysSocketpair, "socketpair"},
    {kSysShutdown,   "shutdown"},
};

[[nodiscard]] inline unsigned socket_syscall_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kSocketSyscallTable)
                                  / sizeof(kSocketSyscallTable[0]));
}

[[nodiscard]] constexpr bool is_socket_syscall(int n) noexcept {
    return n == kSysSocket || n == kSysConnect || n == kSysBind || n == kSysListen
        || n == kSysAccept || n == kSysSendto || n == kSysRecvfrom
        || n == kSysSendmsg || n == kSysRecvmsg || n == kSysGetsockopt
        || n == kSysSetsockopt || n == kSysSocketpair || n == kSysShutdown;
}

[[nodiscard]] constexpr Status validate_domain(int domain) noexcept {
    if (domain == kAfUnspec || domain == kAfLocal || domain == kAfInet
        || domain == kAfInet6) {
        return Status::Ok;
    }
    return Status::Invalid;
}

[[nodiscard]] constexpr Status validate_type(int type) noexcept {
    const int base = type & 0xFF;
    if (base == kSockStream || base == kSockDgram || base == kSockRaw) {
        return Status::Ok;
    }
    return Status::Invalid;
}

[[nodiscard]] constexpr Status validate_socket_triple(int domain, int type,
                                                      int protocol) noexcept {
    if (validate_domain(domain) != Status::Ok) {
        return Status::Invalid;
    }
    if (validate_type(type) != Status::Ok) {
        return Status::Invalid;
    }
    if (protocol < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr bool needs_connected_socket(int n) noexcept {
    return n == kSysConnect || n == kSysSendto || n == kSysSendmsg
        || n == kSysShutdown;
}

} // namespace pbsd::kernel::socket_syscall
