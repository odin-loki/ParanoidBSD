module;
#include <cstddef>

export module pbsd.userland.vipw;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Stub helpers from hbsd/src/usr.sbin/vipw — editor/lock parse scaffold (logic-only).
export namespace pbsd::userland::usr_sbin::vipw {

struct Options {
    const char* editor{nullptr};
    bool quiet{false};
    bool no_lock{false};
    const char* passwd_file{nullptr};
    const char* shadow_file{nullptr};
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        const char* arg = argv[i];
        if (hosted::cstrcmp(arg, "-q") == 0) {
            opt.quiet = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-d") == 0) {
            opt.no_lock = true;
            continue;
        }
        if (arg[1] == 'e' && arg[2] != '\0') {
            opt.editor = arg + 2;
            continue;
        }
        if (arg[1] == 'e' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.editor = argv[++i];
            continue;
        }
        if (arg[1] == 'f' && arg[2] != '\0') {
            opt.passwd_file = arg + 2;
            continue;
        }
        if (arg[1] == 'f' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.passwd_file = argv[++i];
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline const char* default_editor() noexcept {
    return "vi";
}

[[nodiscard]] inline bool passwd_line_valid(const char* line) noexcept {
    if (line == nullptr || line[0] == '\0' || line[0] == '#') {
        return false;
    }
    int colons = 0;
    for (const char* p = line; *p; ++p) {
        if (*p == ':') {
            ++colons;
        }
    }
    return colons >= 6;
}

} // namespace pbsd::userland::usr_sbin::vipw
