module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.ping;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/sbin/ping/ping.c — flag parse stub (logic-only).
export namespace pbsd::userland::sbin::ping {

inline constexpr int kDefDataLen = 56;
inline constexpr int kMaxWait = 10000;

struct Options {
    bool flood{false};
    bool quiet{false};
    bool numeric{false};
    bool verbose{false};
    bool broadcast{false};
    bool dontfrag{false};
    bool ipv4{false};
    bool ipv6{false};
    int count{-1};
    int preload{0};
    int ttl{0};
    int interval_ms{1000};
    int wait_ms{kMaxWait};
    int size{kDefDataLen};
    const char* pattern{nullptr};
    const char* host{nullptr};
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr; ++i) {
        const char* arg = argv[i];
        if (arg[0] != '-') {
            break;
        }
        if (hosted::cstrcmp(arg, "-4") == 0) {
            opt.ipv4 = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-6") == 0) {
            opt.ipv6 = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-A") == 0) {
            opt.broadcast = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-D") == 0) {
            opt.dontfrag = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-f") == 0) {
            opt.flood = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-n") == 0) {
            opt.numeric = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-q") == 0) {
            opt.quiet = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-v") == 0) {
            opt.verbose = true;
            continue;
        }
        if (arg[1] == 'c' && arg[2] != '\0') {
            opt.count = 0;
            for (const char* p = arg + 2; *p >= '0' && *p <= '9'; ++p) {
                opt.count = opt.count * 10 + (*p - '0');
            }
            continue;
        }
        if (arg[1] == 'c' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.count = 0;
            for (const char* p = argv[++i]; *p >= '0' && *p <= '9'; ++p) {
                opt.count = opt.count * 10 + (*p - '0');
            }
            continue;
        }
        if (arg[1] == 's' && arg[2] != '\0') {
            opt.size = 0;
            for (const char* p = arg + 2; *p >= '0' && *p <= '9'; ++p) {
                opt.size = opt.size * 10 + (*p - '0');
            }
            continue;
        }
        if (arg[1] == 'W' && arg[2] != '\0') {
            opt.wait_ms = 0;
            for (const char* p = arg + 2; *p >= '0' && *p <= '9'; ++p) {
                opt.wait_ms = opt.wait_ms * 10 + (*p - '0');
            }
            continue;
        }
        if (arg[1] == 'i' && arg[2] != '\0') {
            opt.interval_ms = 0;
            for (const char* p = arg + 2; *p >= '0' && *p <= '9'; ++p) {
                opt.interval_ms = opt.interval_ms * 10 + (*p - '0');
            }
            continue;
        }
        if (arg[1] == 'm' && arg[2] != '\0') {
            opt.ttl = 0;
            for (const char* p = arg + 2; *p >= '0' && *p <= '9'; ++p) {
                opt.ttl = opt.ttl * 10 + (*p - '0');
            }
            continue;
        }
        if (arg[1] == 'p' && arg[2] != '\0') {
            opt.pattern = arg + 2;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    if (i < argc) {
        opt.host = argv[i];
    }
    if (opt.host == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    if (opt.flood && opt.interval_ms != 1000) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

} // namespace pbsd::userland::sbin::ping
