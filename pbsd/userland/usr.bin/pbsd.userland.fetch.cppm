module;
#include <cstddef>

export module pbsd.userland.fetch;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Stub helpers from hbsd/src/usr.bin/fetch — URL/flag parse scaffold (logic-only).
export namespace pbsd::userland::usr_bin::fetch {

struct Options {
    const char* output{nullptr};
    const char* user{nullptr};
    const char* password{nullptr};
    const char* referer{nullptr};
    const char* user_agent{nullptr};
    bool quiet{false};
    bool verbose{false};
    bool no_redirect{false};
    bool resume{false};
    bool mirror{false};
    int timeout_sec{0};
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
        if (arg[1] == 'o' && arg[2] != '\0') {
            opt.output = arg + 2;
            continue;
        }
        if (arg[1] == 'o' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.output = argv[++i];
            continue;
        }
        if (arg[1] == 'T' && arg[2] != '\0') {
            int n = 0;
            for (const char* p = arg + 2; *p >= '0' && *p <= '9'; ++p) {
                n = n * 10 + (*p - '0');
            }
            opt.timeout_sec = n;
            continue;
        }
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'q':
                opt.quiet = true;
                break;
            case 'v':
                opt.verbose = true;
                break;
            case 'R':
                opt.no_redirect = true;
                break;
            case 'r':
                opt.resume = true;
                break;
            case 'm':
                opt.mirror = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline bool url_has_scheme(const char* url) noexcept {
    if (url == nullptr) {
        return false;
    }
    const char* colon = nullptr;
    for (const char* p = url; *p && *p != '/'; ++p) {
        if (*p == ':') {
            colon = p;
            break;
        }
    }
    return colon != nullptr && colon > url;
}

} // namespace pbsd::userland::usr_bin::fetch
