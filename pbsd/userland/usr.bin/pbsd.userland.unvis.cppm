module;
#include <cstddef>

export module pbsd.userland.unvis;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/contrib/unvis/unvis.c — visibility decode flags (logic-only).
export namespace pbsd::userland::usr_bin::unvis {

inline constexpr int kVisNoescape = 0x100;
inline constexpr int kVisHttp1808 = 0x200;
inline constexpr int kVisHttp1866 = 0x400;
inline constexpr int kVisMimestyle = 0x800;

struct Options {
    int eflags{0};
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
            switch (arg[j]) {
            case 'e':
                opt.eflags |= kVisNoescape;
                break;
            case 'H':
                opt.eflags |= kVisHttp1866;
                break;
            case 'h':
                opt.eflags |= kVisHttp1808;
                break;
            case 'm':
                opt.eflags |= kVisMimestyle;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline bool http_flags_conflict(int eflags) noexcept {
    const int http = kVisHttp1808 | kVisHttp1866 | kVisMimestyle;
    int count = 0;
    if ((eflags & kVisHttp1808) != 0) {
        ++count;
    }
    if ((eflags & kVisHttp1866) != 0) {
        ++count;
    }
    if ((eflags & kVisMimestyle) != 0) {
        ++count;
    }
    return count > 1;
}

} // namespace pbsd::userland::usr_bin::unvis
