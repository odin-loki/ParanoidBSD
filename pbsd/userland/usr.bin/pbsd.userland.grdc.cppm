module;
#include <cstddef>

export module pbsd.userland.grdc;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/grdc/grdc.c — seven-segment digit patterns (logic-only).
export namespace pbsd::userland::usr_bin::grdc {

inline constexpr int kYBase = 10;
inline constexpr int kXBase = 10;
inline constexpr int kXLength = 58;
inline constexpr int kYDepth = 7;

inline constexpr short kDigitSegments[11] = {
    075557, 011111, 071747, 071717, 055711,
    074717, 074757, 071111, 075757, 075717, 002020,
};

struct Options {
    bool scroll{false};
    bool twelve_hour{false};
    bool countdown{false};
    int seconds{0};
};

[[nodiscard]] inline Result<int> parse_duration(const char* s) noexcept {
    if (s == nullptr || *s == '\0') {
        return result_err<int>(Status::Invalid);
    }
    int val = 0;
    for (const char* p = s; *p != '\0'; ++p) {
        if (*p < '0' || *p > '9') {
            return result_err<int>(Status::Invalid);
        }
        val = val * 10 + (*p - '0');
    }
    if (val < 1) {
        return result_err<int>(Status::Invalid);
    }
    return result_ok(val + 1);
}

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
            switch (arg[j]) {
            case 'c':
                opt.countdown = true;
                break;
            case 's':
                opt.scroll = true;
                break;
            case 't':
                opt.twelve_hour = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    if (opt.countdown && argc - i != 1) {
        return result_err<Options>(Status::Invalid);
    }
    if (argc - i > 1) {
        return result_err<Options>(Status::Invalid);
    }
    if (argc - i == 1) {
        auto parsed = parse_duration(argv[i]);
        if (parsed.status != Status::Ok) {
            return result_err<Options>(Status::Invalid);
        }
        opt.seconds = parsed.value;
    }
    return result_ok(opt);
}

[[nodiscard]] inline int to_twelve_hour(int hour) noexcept {
    if (hour > 11) {
        return hour - 12;
    }
    return hour;
}

[[nodiscard]] inline short segment_mask(int digit) noexcept {
    if (digit < 0 || digit > 10) {
        return 0;
    }
    return kDigitSegments[digit];
}

} // namespace pbsd::userland::usr_bin::grdc
