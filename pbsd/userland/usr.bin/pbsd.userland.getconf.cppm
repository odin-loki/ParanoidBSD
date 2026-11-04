module;
#include <cstddef>

export module pbsd.userland.getconf;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/getconf/getconf.c — configuration query flags (logic-only).
export namespace pbsd::userland::usr_bin::getconf {

struct Options {
    bool list_all{false};
    const char* prog_env{nullptr};
    const char* var_name{nullptr};
    const char* path_name{nullptr};
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
        if (hosted::cstrcmp(arg, "-a") == 0) {
            opt.list_all = true;
            continue;
        }
        if (arg[1] == 'v' && arg[2] != '\0') {
            opt.prog_env = arg + 2;
            continue;
        }
        if (hosted::cstrcmp(arg, "-v") == 0) {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.prog_env = argv[++i];
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    if (opt.list_all) {
        if (opt.prog_env != nullptr) {
            return result_err<Options>(Status::Invalid);
        }
        if (argc - i > 1) {
            return result_err<Options>(Status::Invalid);
        }
        if (argc - i == 1) {
            opt.path_name = argv[i];
        }
        return result_ok(opt);
    }
    if (argc - i < 1) {
        return result_err<Options>(Status::Invalid);
    }
    opt.var_name = argv[i];
    if (argc - i >= 2) {
        opt.path_name = argv[i + 1];
    }
    return result_ok(opt);
}

[[nodiscard]] inline bool needs_path(const char* name) noexcept {
    if (name == nullptr) {
        return false;
    }
    static constexpr const char* path_vars[] = {
        "LINK_MAX", "MAX_CANON", "MAX_INPUT", "NAME_MAX", "PATH_MAX",
        "PIPE_BUF", "POSIX_VDISABLE", "_POSIX_CHOWN_RESTRICTED",
        "_POSIX_NO_TRUNC", "_POSIX_VDISABLE",
    };
    for (const char* var : path_vars) {
        if (hosted::cstrcmp(name, var) == 0) {
            return true;
        }
    }
    return false;
}

} // namespace pbsd::userland::usr_bin::getconf
