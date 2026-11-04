module;

#if defined(__FreeBSD__)
#include <sys/capsicum.h>
#include <cerrno>
#endif

export module pbsd.userland.capsicum.helpers;

export import pbsd.userland.capsicum;
import pbsd.core;

/// Real port of hbsd/src/lib/libcapsicum/capsicum_helpers.h stream helpers.
export namespace pbsd::userland::capsicum {

enum class StreamLimitFlags : unsigned {
    None         = 0,
    IgnoreEbadf  = 1u << 0,
    Read         = 1u << 1,
    Write        = 1u << 2,
    Lookup       = 1u << 3,
};

[[nodiscard]] constexpr StreamLimitFlags operator|(StreamLimitFlags a,
                                                     StreamLimitFlags b) noexcept {
    return static_cast<StreamLimitFlags>(static_cast<unsigned>(a) |
                                         static_cast<unsigned>(b));
}

[[nodiscard]] inline CapabilityRights stream_rights(StreamLimitFlags flags) noexcept {
    CapabilityRights r = CapabilityRights::Read | CapabilityRights::Write;
    if ((static_cast<unsigned>(flags) &
         static_cast<unsigned>(StreamLimitFlags::Read)) != 0u) {
        r = r | CapabilityRights::Read;
    }
    if ((static_cast<unsigned>(flags) &
         static_cast<unsigned>(StreamLimitFlags::Write)) != 0u) {
        r = r | CapabilityRights::Write;
    }
    if ((static_cast<unsigned>(flags) &
         static_cast<unsigned>(StreamLimitFlags::Lookup)) != 0u) {
        r = r | CapabilityRights::Map;
    }
    return r;
}

/// Capsicum-first fd wrapper: rights are narrowed before any syscall surface.
struct FdObject {
    int fd{-1};

    static void release(FdObject* p) noexcept {
        if (p != nullptr) {
            p->fd = -1;
        }
    }
};

using FdHandle = UniqueHandle<FdObject>;

[[nodiscard]] inline Result<FdHandle> adopt_fd(int fd, CapabilityRights rights,
                                                 LineageId lineage) noexcept {
    if (fd < 0) {
        return {Status::Invalid, FdHandle{}};
    }
    return {Status::Ok, FdHandle{new FdObject{fd}, rights, lineage}};
}

/// caph_stream_rights equivalent — builds rights set for stream limiting.
[[nodiscard]] inline os::CapRights cap_stream_rights(StreamLimitFlags flags) noexcept {
    auto rights = os::CapRights::init_base();
#if defined(__FreeBSD__)
    if ((static_cast<unsigned>(flags) &
         static_cast<unsigned>(StreamLimitFlags::Read)) != 0u) {
        rights.add(CAP_READ);
    }
    if ((static_cast<unsigned>(flags) &
         static_cast<unsigned>(StreamLimitFlags::Write)) != 0u) {
        rights.add(CAP_WRITE);
    }
    if ((static_cast<unsigned>(flags) &
         static_cast<unsigned>(StreamLimitFlags::Lookup)) != 0u) {
        rights.add(CAP_LOOKUP);
    }
#else
    (void)flags;
#endif
    return rights;
}

/// caph_limit_stream — real rights/ioctl/fcntl narrowing.
[[nodiscard]] inline Status limit_stream(int fd, StreamLimitFlags flags) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    const auto rights = cap_stream_rights(flags);
    const auto rl = os::rights_limit(fd, rights);
    if (!ok(rl.status)) {
#if defined(__FreeBSD__)
        if (errno == EBADF &&
            (static_cast<unsigned>(flags) &
             static_cast<unsigned>(StreamLimitFlags::IgnoreEbadf)) != 0u) {
            return Status::Ok;
        }
#endif
        return rl.status;
    }
    const auto io = os::ioctls_limit(fd);
    if (!ok(io.status)) {
        return io.status;
    }
    const auto fc = os::fcntls_limit(fd);
    return fc.status;
}

[[nodiscard]] inline Status limit_stream(FdHandle& fd, StreamLimitFlags flags) noexcept {
    if (!fd.valid()) {
        return Status::Invalid;
    }
    const CapabilityRights want = stream_rights(flags);
    if (!fd.has_right(want)) {
        return Status::Denied;
    }
    return limit_stream(fd.peek()->fd, flags);
}

[[nodiscard]] inline Status limit_stdin() noexcept {
    return limit_stream(0, StreamLimitFlags::Read);
}

[[nodiscard]] inline Status limit_stdout() noexcept {
    return limit_stream(1, StreamLimitFlags::Write);
}

[[nodiscard]] inline Status limit_stderr() noexcept {
    return limit_stream(2, StreamLimitFlags::Write);
}

/// caph_limit_stdio
[[nodiscard]] inline Status limit_stdio() noexcept {
    const auto iebadf = StreamLimitFlags::IgnoreEbadf;
    if (limit_stream(0, StreamLimitFlags::Read | iebadf) != Status::Ok ||
        limit_stream(1, StreamLimitFlags::Write | iebadf) != Status::Ok ||
        limit_stream(2, StreamLimitFlags::Write | iebadf) != Status::Ok) {
        return Status::Protocol;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status limit_stdio(LineageTree& tree) noexcept {
    const auto id = tree.create_root();
    if (id == kInvalidLineage) {
        return Status::NoMemory;
    }
    return limit_stdio();
}

/// caph_enter
[[nodiscard]] inline Status enter_sandbox() noexcept {
    const auto st = capsicum::enter();
    return st.status;
}

} // namespace pbsd::userland::capsicum
