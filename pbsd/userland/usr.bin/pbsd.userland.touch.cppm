module;
#include <cstdint>

export module pbsd.userland.touch;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/touch/touch.c — flag parse (logic-only).
export namespace pbsd::userland::usr_bin::touch {

inline constexpr int kAtSymlinkNofollow = 0x2000;

struct Timespec {
    std::int64_t tv_sec{0};
    std::int64_t tv_nsec{0};
};

inline constexpr std::int64_t kUtimeNow = -1;

struct Options {
    bool access_time{false};
    bool mod_time{false};
    bool create{false};
    bool timeset{false};
    int offset_secs{0};
    Timespec ts[2]{{0, kUtimeNow}, {0, kUtimeNow}};
    int atflag{0};
};

[[nodiscard]] inline Result<int> timeoffset(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return result_err<int>(Status::Invalid);
    }
    int sign = 1;
    const char* p = arg;
    if (*p == '+') {
        ++p;
    } else if (*p == '-') {
        sign = -1;
        ++p;
    }
    long val = 0;
    for (; *p != '\0'; ++p) {
        if (*p < '0' || *p > '9') {
            return result_err<int>(Status::Invalid);
        }
        val = val * 10 + (*p - '0');
    }
    return result_ok(static_cast<int>(sign * val));
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    while (i < argc && argv[i] != nullptr && argv[i][0] == '-') {
        const char* flag = argv[i];
        if (flag[1] == 'A' && flag[2] != '\0') {
            const auto off = timeoffset(flag + 2);
            if (!off.has_value()) {
                return result_err<Options>(off.status);
            }
            opt.offset_secs = off.value;
            ++i;
            continue;
        }
        for (int j = 1; flag[j] != '\0'; ++j) {
            switch (flag[j]) {
            case 'a':
                opt.access_time = true;
                break;
            case 'c':
                opt.create = true;
                break;
            case 'f':
                break;
            case 'h':
                opt.create = true;
                opt.atflag = kAtSymlinkNofollow;
                break;
            case 'm':
                opt.mod_time = true;
                break;
            case 'd':
            case 'r':
            case 't':
                opt.timeset = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
        ++i;
    }
    optind_out = i;
    if (!opt.access_time && !opt.mod_time) {
        opt.access_time = true;
        opt.mod_time = true;
    }
    if (opt.offset_secs != 0) {
        opt.create = true;
    }
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::touch
