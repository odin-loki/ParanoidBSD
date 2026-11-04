module;
#include <cstddef>
#include <cstdint>

export module pbsd.kernel.syscall_args;

export import pbsd.core;

/// Freestanding syscall argument layouts (from hbsd/src/sys/sys/sysproto.h).
export namespace pbsd::kernel::syscall_args {

struct ReadArgs {
    int         fd{};
    void*       buf{};
    std::size_t nbyte{};
};

struct WriteArgs {
    int               fd{};
    const void*       buf{};
    std::size_t       nbyte{};
};

struct OpenArgs {
    const char* path{};
    int         flags{};
    std::uint32_t mode{};
};

struct CloseArgs {
    int fd{};
};

struct Pipe2Args {
    int* fildes{};
    int  flags{};
};

struct PollArgs {
    void*     fds{};
    unsigned  nfds{};
    int       timeout{};
};

struct KqueueArgs {
    int dummy{};
};

struct KeventArgs {
    int                   fd{};
    const void*           changelist{};
    int                   nchanges{};
    void*                 eventlist{};
    int                   nevents{};
    const void*           timeout{};
};

struct SocketArgs {
    int domain{};
    int type{};
    int protocol{};
};

struct ConnectArgs {
    int               s{};
    const void*       name{};
    std::uint32_t     namelen{};
};

struct BindArgs {
    int               s{};
    const void*       name{};
    std::uint32_t     namelen{};
};

struct ListenArgs {
    int s{};
    int backlog{};
};

struct AcceptArgs {
    int     s{};
    void*   name{};
    void*   namelen{};
};

struct SendtoArgs {
    int               s{};
    const void*       buf{};
    std::size_t       len{};
    int               flags{};
    const void*       to{};
    std::uint32_t     tolen{};
};

struct RecvfromArgs {
    int         s{};
    void*       buf{};
    std::size_t len{};
    int         flags{};
    void*       from{};
    void*       fromlen{};
};

struct MkfifoArgs {
    const char*   path{};
    std::uint32_t mode{};
};

struct SyscallArgEntry {
    const char* name;
    unsigned    field_count;
};

inline constexpr SyscallArgEntry kArgTable[] = {
    {"read",      3},
    {"write",     3},
    {"open",      3},
    {"close",     1},
    {"pipe2",     2},
    {"poll",      3},
    {"kqueue",    0},
    {"kevent",    6},
    {"socket",    3},
    {"connect",   3},
    {"bind",      3},
    {"listen",    2},
    {"accept",    3},
    {"sendto",    6},
    {"recvfrom",  6},
    {"mkfifo",    2},
};

[[nodiscard]] inline unsigned arg_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kArgTable) / sizeof(kArgTable[0]));
}

[[nodiscard]] inline Result<unsigned> field_count(const char* name) noexcept {
    if (name == nullptr) {
        return result_err<unsigned>(Status::Invalid);
    }
    for (const auto& e : kArgTable) {
        const char* a = name;
        const char* b = e.name;
        while (*a && *b && *a == *b) {
            ++a;
            ++b;
        }
        if (*a == '\0' && *b == '\0') {
            return result_ok(e.field_count);
        }
    }
    return result_err<unsigned>(Status::NotFound);
}

[[nodiscard]] constexpr bool validate_read(const ReadArgs& a) noexcept {
    return a.fd >= 0 && a.buf != nullptr && a.nbyte > 0;
}

[[nodiscard]] constexpr bool validate_open(const OpenArgs& a) noexcept {
    return a.path != nullptr;
}

[[nodiscard]] constexpr bool validate_poll(const PollArgs& a) noexcept {
    return a.nfds == 0 || a.fds != nullptr;
}

[[nodiscard]] constexpr bool validate_kevent(const KeventArgs& a) noexcept {
    if (a.nchanges < 0 || a.nevents < 0) {
        return false;
    }
    if (a.nchanges > 0 && a.changelist == nullptr) {
        return false;
    }
    if (a.nevents > 0 && a.eventlist == nullptr) {
        return false;
    }
    return true;
}

[[nodiscard]] constexpr bool validate_socket(const SocketArgs& a) noexcept {
    return a.domain >= 0 && a.type >= 0;
}

} // namespace pbsd::kernel::syscall_args
