module;
#include <cstddef>

export module pbsd.userland.lsvfs;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/lsvfs/lsvfs.c — VFS flag formatting (logic-only).
export namespace pbsd::userland::usr_bin::lsvfs {

struct VfsFlag {
    int flag;
    const char* name;
};

inline constexpr VfsFlag kFlags[] = {
    {0x0001, "static"},
    {0x0002, "network"},
    {0x0004, "read-only"},
    {0x0008, "synthetic"},
    {0x0010, "loopback"},
    {0x0020, "unicode"},
    {0x0040, "jail"},
    {0x0080, "delegated-administration"},
};

[[nodiscard]] inline std::size_t format_flags(int flags, char* buf, std::size_t len) noexcept {
    if (buf == nullptr || len == 0) {
        return 0;
    }
    std::size_t pos = 0;
    for (const VfsFlag& f : kFlags) {
        if ((flags & f.flag) == 0) {
            continue;
        }
        if (pos != 0) {
            if (pos + 2 >= len) {
                break;
            }
            buf[pos++] = ',';
            buf[pos++] = ' ';
        }
        const char* p = f.name;
        while (*p != '\0' && pos + 1 < len) {
            buf[pos++] = *p++;
        }
    }
    if (pos < len) {
        buf[pos] = '\0';
    }
    return pos;
}

[[nodiscard]] inline Result<int> parse_args(int argc, char* const* argv,
                                            int& optind_out) noexcept {
    if (argv == nullptr) {
        return result_err<int>(Status::Invalid);
    }
    optind_out = 1;
    return result_ok(argc > 1 ? 0 : 0);
}

} // namespace pbsd::userland::usr_bin::lsvfs
