module;
#include <cstddef>

export module pbsd.userland.vis;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/vis/vis.c — visibility encode flags (logic-only).
export namespace pbsd::userland::usr_bin::vis {

inline constexpr int kVisCstyle = 0x01;
inline constexpr int kVisOctal = 0x02;
inline constexpr int kVisNoslash = 0x04;

struct Options {
    int flags{0};
};

[[nodiscard]] inline bool needs_escape(unsigned char c, int flags) noexcept {
    if (c >= 0x20 && c < 0x7f && c != '\\') {
        return false;
    }
    if ((flags & kVisCstyle) != 0 && (c == '\n' || c == '\r' || c == '\t')) {
        return true;
    }
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
        const char* arg = argv[i];
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'c':
                opt.flags |= kVisCstyle;
                break;
            case 'o':
                opt.flags |= kVisOctal;
                break;
            case 'F':
                opt.flags |= kVisNoslash;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::vis
