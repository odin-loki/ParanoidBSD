module;

export module pbsd.userland.libc.net.gai_strerror;

export import pbsd.core;

/// gai_strerror from hbsd/src/lib/libc/net/gai_strerror.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline const char* gai_strerror_msg(int ecode) noexcept {
    switch (ecode) {
    case 1: return "Temporary failure in name resolution";
    case 2: return "Non-recoverable failure in name resolution";
    case 3: return "Invalid flags";
    default: return "Unknown error";
    }
}

} // namespace pbsd::userland::libc
