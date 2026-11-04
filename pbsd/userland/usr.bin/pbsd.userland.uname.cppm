module;
#include <cstdint>

export module pbsd.userland.uname;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/uname/uname.c — flag parse and field selection.
export namespace pbsd::userland::usr_bin::uname {

inline constexpr unsigned kSysname  = 0x0010;
inline constexpr unsigned kNodename = 0x0002;
inline constexpr unsigned kRelease  = 0x0008;
inline constexpr unsigned kVersion  = 0x0020;
inline constexpr unsigned kMachine  = 0x0001;
inline constexpr unsigned kPlatform = 0x0004;
inline constexpr unsigned kIdent    = 0x0040;
inline constexpr unsigned kKernvers = 0x0100;
inline constexpr unsigned kUservers = 0x0080;
inline constexpr unsigned kBuildid  = 0x0200;

inline constexpr unsigned kAll = kMachine | kNodename | kRelease | kSysname | kVersion;

struct Fields {
    const char* sysname{nullptr};
    const char* nodename{nullptr};
    const char* release{nullptr};
    const char* version{nullptr};
    const char* machine{nullptr};
    const char* platform{nullptr};
    const char* ident{nullptr};
    const char* kernvers{nullptr};
    const char* uservers{nullptr};
    const char* buildid{nullptr};
};

[[nodiscard]] inline Result<unsigned> parse_args(int argc, char* const* argv,
                                                 int& optind_out) noexcept {
    unsigned flags = 0;
    if (argv == nullptr) {
        return result_err<unsigned>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        for (const char* p = argv[i] + 1; *p != '\0'; ++p) {
            switch (*p) {
            case 'a':
                flags |= kAll;
                break;
            case 'b':
                flags |= kBuildid;
                break;
            case 'i':
                flags |= kIdent;
                break;
            case 'K':
                flags |= kKernvers;
                break;
            case 'm':
                flags |= kMachine;
                break;
            case 'n':
                flags |= kNodename;
                break;
            case 'p':
                flags |= kPlatform;
                break;
            case 'r':
                flags |= kRelease;
                break;
            case 's':
            case 'o':
                flags |= kSysname;
                break;
            case 'U':
                flags |= kUservers;
                break;
            case 'v':
                flags |= kVersion;
                break;
            default:
                return result_err<unsigned>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    if (i < argc) {
        return result_err<unsigned>(Status::Invalid);
    }
    if (flags == 0) {
        flags = kSysname;
    }
    return result_ok(flags);
}

[[nodiscard]] inline std::size_t format_fields(char* buf, std::size_t cap,
                                               unsigned flags,
                                               const Fields& f) noexcept {
    if (buf == nullptr || cap == 0) {
        return 0;
    }
    std::size_t pos = 0;
    auto append = [&](const char* s) {
        if (s == nullptr) {
            return;
        }
        if (pos > 0 && pos + 1 < cap) {
            buf[pos++] = ' ';
        }
        for (const char* p = s; *p != '\0' && pos + 1 < cap; ++p) {
            buf[pos++] = *p;
        }
    };
    if ((flags & kSysname) != 0) {
        append(f.sysname);
    }
    if ((flags & kNodename) != 0) {
        append(f.nodename);
    }
    if ((flags & kRelease) != 0) {
        append(f.release);
    }
    if ((flags & kVersion) != 0) {
        append(f.version);
    }
    if ((flags & kMachine) != 0) {
        append(f.machine);
    }
    if ((flags & kPlatform) != 0) {
        append(f.platform);
    }
    if ((flags & kIdent) != 0) {
        append(f.ident);
    }
    if ((flags & kKernvers) != 0) {
        append(f.kernvers);
    }
    if ((flags & kUservers) != 0) {
        append(f.uservers);
    }
    if ((flags & kBuildid) != 0) {
        append(f.buildid);
    }
    if (pos < cap) {
        buf[pos++] = '\n';
    }
    return pos;
}

[[nodiscard]] inline void sanitize_version(char* buf, std::size_t len) noexcept {
    if (buf == nullptr) {
        return;
    }
    for (std::size_t n = 0; n < len; ++n) {
        if (buf[n] == '\n' || buf[n] == '\t') {
            buf[n] = (n + 1 < len) ? ' ' : '\0';
        }
    }
}

} // namespace pbsd::userland::usr_bin::uname
