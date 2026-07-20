module;

export module pbsd.userland.stty;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/bin/stty/{stty.c,stty.h} — format modes and flags.
export namespace pbsd::userland::bin::stty {

inline constexpr int kLineLength = 72;
inline constexpr const char* kOptString = "aef:g";

enum class OutputFormat : unsigned char {
    NotSet,
    GFlag,
    Bsd,
    Posix,
};

struct Options {
    OutputFormat format{OutputFormat::NotSet};
    const char* device{nullptr};
    int fd{0};
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
            case 'a':
                opt.format = OutputFormat::Posix;
                break;
            case 'e':
                opt.format = OutputFormat::Bsd;
                break;
            case 'g':
                opt.format = OutputFormat::GFlag;
                break;
            case 'f':
                ++j;
                while (arg[j] != '\0') {
                    ++j;
                }
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

} // namespace pbsd::userland::bin::stty
