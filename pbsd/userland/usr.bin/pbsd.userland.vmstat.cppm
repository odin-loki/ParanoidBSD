module;
#include <cstddef>

export module pbsd.userland.vmstat;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/vmstat/vmstat.c — Interval/count parse
export namespace pbsd::userland::usr_bin::vmstat {

struct Options {
    int interval{0};
    int count{-1};
    bool wide{false};
};

[[nodiscard]] inline bool parse_count(const char* s, int& out) noexcept {
    if (s == nullptr) {
        return false;
    }
    int n = 0;
    for (const char* p = s; *p >= '0' && *p <= '9'; ++p) {
        n = n * 10 + (*p - '0');
    }
    out = n;
    return true;
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        if (hosted::cstrcmp(argv[i], "-w") == 0) {
            opt.wide = true;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    if (i < argc && argv[i] != nullptr && argv[i][0] != '-') {
        parse_count(argv[i], opt.interval);
        ++i;
    }
    if (i < argc && argv[i] != nullptr && argv[i][0] != '-') {
        parse_count(argv[i], opt.count);
        ++i;
    }
    optind_out = i;
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::vmstat
