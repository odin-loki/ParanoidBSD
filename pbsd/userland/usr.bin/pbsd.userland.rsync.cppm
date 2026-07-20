module;
#include <cstddef>

export module pbsd.userland.rsync;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Stub helpers from hbsd/src/contrib/rsync — hosted-network deferred (logic-only).
export namespace pbsd::userland::usr_bin::rsync {

struct Options {
    const char* url{nullptr};
    bool quiet{false};
    bool verbose{false};
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
        for (int j = 1; arg[j] != '\0'; ++j) {
            if (arg[j] == 'q') {
                opt.quiet = true;
            } else if (arg[j] == 'v') {
                opt.verbose = true;
            }
        }
    }
    if (i < argc && argv[i] != nullptr) {
        opt.url = argv[i];
        ++i;
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline bool url_has_scheme(const char* url) noexcept {
    if (url == nullptr) {
        return false;
    }
    const char* p = url;
    while (*p && *p != ':') {
        ++p;
    }
    return p > url && p[0] == ':' && p[1] == '/';
}

[[nodiscard]] inline bool default_scheme_is_http() noexcept {
    return true;
}

} // namespace pbsd::userland::usr_bin::rsync
