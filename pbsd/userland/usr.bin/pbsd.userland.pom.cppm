module;
#include <cstddef>

export module pbsd.userland.pom;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/pom/pom.c — phase-of-moon constants (logic-only).
export namespace pbsd::userland::usr_bin::pom {

inline constexpr double kPi = 3.14159265358979323846;
inline constexpr int kEpoch = 85;
inline constexpr double kEpsilonG = 279.611371;
inline constexpr double kRhoG = 282.680403;
inline constexpr double kEccen = 0.01671542;
inline constexpr double kLzero = 18.251907;
inline constexpr double kPzero = 192.917585;
inline constexpr double kNzero = 55.204723;

struct Options {
    bool plain_output{false};
    int repeat_count{1};
};

[[nodiscard]] inline bool is_leap_year(int year) noexcept {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        if (hosted::cstrcmp(argv[i], "-p") == 0) {
            opt.plain_output = true;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    if (argc - i == 1) {
        const char* count = argv[i];
        int val = 0;
        for (const char* p = count; *p != '\0'; ++p) {
            if (*p < '0' || *p > '9') {
                return result_err<Options>(Status::Invalid);
            }
            val = val * 10 + (*p - '0');
        }
        if (val < 1) {
            return result_err<Options>(Status::Invalid);
        }
        opt.repeat_count = val;
    } else if (argc - i > 1) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

[[nodiscard]] inline double deg_to_rad(double deg) noexcept {
    return deg * kPi / 180.0;
}

inline void adjust_360(double& angle) noexcept {
    while (angle >= 360.0) {
        angle -= 360.0;
    }
    while (angle < 0.0) {
        angle += 360.0;
    }
}

} // namespace pbsd::userland::usr_bin::pom
