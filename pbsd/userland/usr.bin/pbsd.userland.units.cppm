module;
#include <cstddef>

export module pbsd.userland.units;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/units/units.c — unit conversion.
export namespace pbsd::userland::usr_bin::units {

inline constexpr const char kUnitsFile[] = "/usr/share/misc/definitions.units";
inline constexpr int kMaxUnits = 1000;
inline constexpr int kMaxPrefixes = 100;
inline constexpr int kMaxSubUnits = 500;
inline constexpr char kPrimitiveChar = '!';
inline constexpr char kSeparator = ':';
inline constexpr const char kDefaultNumFmt[] = "%.8g";
inline constexpr const char kScientificFmt[] = "%6e";

struct Options {
    bool verbose{false};
    bool terse{false};
    bool quiet{false};
    bool readfile{false};
    const char* outputformat{kDefaultNumFmt};
    const char* havestr{nullptr};
    const char* wantstr{nullptr};
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
        if (hosted::cstrcmp(arg, "--") == 0) {
            ++i;
            break;
        }
        if (arg[1] == '-' && arg[2] != '\0') {
            if (hosted::cstrcmp(arg, "--terse") == 0) {
                opt.terse = true;
            } else if (hosted::cstrcmp(arg, "--verbose") == 0) {
                opt.verbose = true;
            }
            continue;
        }
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'e':
                opt.outputformat = kScientificFmt;
                break;
            case 'f':
            case 'o':
                while (arg[j + 1] != '\0') {
                    ++j;
                }
                opt.readfile = true;
                break;
            case 'q':
                opt.quiet = true;
                break;
            case 't':
                opt.terse = true;
                break;
            case 'v':
                opt.verbose = true;
                break;
            case 'H':
            case 'U':
            case 'V':
                break;
            default:
                break;
            }
        }
    }
    if (i < argc && argv[i] != nullptr) {
        opt.havestr = argv[i++];
    }
    if (i < argc && argv[i] != nullptr) {
        opt.wantstr = argv[i++];
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline bool accepts_flag(char c) noexcept {
    const char* allowed = "ehf:o:qtvH:UV";
    for (const char* p = allowed; *p; ++p) {
        if (*p == c) {
            return true;
        }
    }
    return false;
}

} // namespace pbsd::userland::usr_bin::units
