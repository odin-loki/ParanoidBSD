module;
#include <cstddef>

export module pbsd.userland.realpath;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/realpath/realpath.c — option parsing (logic-only).
export namespace pbsd::userland::bin::realpath {

struct Options {
    bool quiet{false};
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }

    int i = 1;
    while (i < argc && argv[i] != nullptr && argv[i][0] == '-') {
        const char* flag = argv[i];
        for (int j = 1; flag[j] != '\0'; ++j) {
            if (flag[j] == 'q') {
                opt.quiet = true;
            } else {
                return result_err<Options>(Status::Invalid);
            }
        }
        ++i;
    }
    return result_ok(opt);
}

[[nodiscard]] inline const char* default_path(int argc, int optind,
                                              char* const* argv) noexcept {
    if (argv == nullptr || optind >= argc || argv[optind] == nullptr) {
        return ".";
    }
    return argv[optind];
}

[[nodiscard]] inline bool path_list_done(int argc, int index) noexcept {
    return index >= argc;
}

} // namespace pbsd::userland::bin::realpath
