module;
#include <cstddef>

export module pbsd.userland.time_cmd;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/time/time.c — flag parse (logic-only).
/// Module name avoids C++ keyword `time`.
export namespace pbsd::userland::usr_bin::time_ {

struct Options {
    bool append_rusage{false};
    bool human{false};
    bool use_rusage{false};
    bool portable{false};
    const char* output_file{nullptr};
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        for (const char* p = argv[i] + 1; *p != '\0'; ++p) {
            switch (*p) {
            case 'a':
                opt.append_rusage = true;
                break;
            case 'h':
                opt.human = true;
                break;
            case 'l':
                opt.use_rusage = true;
                break;
            case 'p':
                opt.portable = true;
                break;
            case 'o':
                ++p;
                if (*p == '\0') {
                    if (i + 1 >= argc) {
                        return result_err<Options>(Status::Invalid);
                    }
                    opt.output_file = argv[++i];
                    break;
                }
                opt.output_file = p;
                p += hosted::cstrlen(p) - 1;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    if (i >= argc) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

[[nodiscard]] inline void format_human(char* buf, std::size_t cap,
                                       long secs, long usecs,
                                       char decimal_point) noexcept {
    if (buf == nullptr || cap == 0) {
        return;
    }
    const long total_ms = secs * 1000L + usecs / 1000L;
    unsigned pos = 0;
    auto put = [&](char c) {
        if (pos + 1 < cap) {
            buf[pos++] = c;
        }
    };
    long n = total_ms;
    char tmp[32];
    unsigned tpos = 0;
    if (n == 0) {
        tmp[tpos++] = '0';
    } else {
        while (n > 0) {
            tmp[tpos++] = static_cast<char>('0' + (n % 10));
            n /= 10;
        }
    }
    while (tpos > 0) {
        put(tmp[--tpos]);
    }
    put(decimal_point);
    const long frac = (usecs % 1000000L) / 100000L;
    put(static_cast<char>('0' + frac));
    put('s');
    if (pos < cap) {
        buf[pos] = '\0';
    }
}

} // namespace pbsd::userland::usr_bin::time_
