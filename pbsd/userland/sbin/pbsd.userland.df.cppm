module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.df;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/df/df.c — flag parsing and statfs math (logic-only).
export namespace pbsd::userland::sbin::df {

inline constexpr int kUnitsSi = 1;
inline constexpr int kUnits2 = 2;

enum class BlockSizeMode : unsigned char {
    Default,
    Kilo,
    Mega,
    Giga,
    Posix512,
};

struct Options {
    bool all{false};
    bool show_inodes{false};
    bool local_only{false};
    bool no_sync{false};
    bool print_type{false};
    bool thousands_sep{false};
    int human_units{0};
    BlockSizeMode blocksize{BlockSizeMode::Default};
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }

    int i = 1;
    while (i < argc && argv[i] != nullptr && argv[i][0] == '-') {
        const char* flag = argv[i];
        if (flag[1] == '-' && flag[2] == '\0') {
            ++i;
            break;
        }
        if (hosted::cstrcmp(flag, "--si") == 0) {
            opt.human_units = kUnitsSi;
            ++i;
            continue;
        }
        for (int j = 1; flag[j] != '\0'; ++j) {
            switch (flag[j]) {
            case 'a':
                opt.all = true;
                break;
            case 'b':
            case 'P':
                if (opt.blocksize != BlockSizeMode::Kilo) {
                    opt.blocksize = BlockSizeMode::Posix512;
                }
                opt.human_units = 0;
                break;
            case 'c':
                break;
            case 'g':
                opt.blocksize = BlockSizeMode::Giga;
                opt.human_units = 0;
                break;
            case 'H':
                opt.human_units = kUnitsSi;
                break;
            case 'h':
                opt.human_units = kUnits2;
                break;
            case 'i':
                opt.show_inodes = true;
                break;
            case 'k':
                opt.blocksize = BlockSizeMode::Kilo;
                break;
            case 'l':
                opt.local_only = true;
                break;
            case 'm':
                opt.blocksize = BlockSizeMode::Mega;
                opt.human_units = 0;
                break;
            case 'n':
                opt.no_sync = true;
                break;
            case 'T':
                opt.print_type = true;
                break;
            case ',':
                opt.thousands_sep = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
        ++i;
    }
    return result_ok(opt);
}

[[nodiscard]] inline int imax(int a, int b) noexcept {
    return a > b ? a : b;
}

[[nodiscard]] inline int int64width(std::int64_t val) noexcept {
    int width = 1;
    if (val < 0) {
        val = -val;
        ++width;
    }
    while (val >= 10) {
        val /= 10;
        ++width;
    }
    return width;
}

[[nodiscard]] inline std::int64_t fsbtoblk(std::int64_t blocks, std::uint64_t fsbsize,
                                             unsigned devbsize) noexcept {
    if (devbsize == 0 || fsbsize == 0) {
        return 0;
    }
    return (blocks * static_cast<std::int64_t>(fsbsize)) /
           static_cast<std::int64_t>(devbsize);
}

[[nodiscard]] inline unsigned blocksize_env(const Options& opt) noexcept {
    switch (opt.blocksize) {
    case BlockSizeMode::Kilo:
        return 1024u;
    case BlockSizeMode::Mega:
        return 1024u * 1024u;
    case BlockSizeMode::Giga:
        return 1024u * 1024u * 1024u;
    case BlockSizeMode::Posix512:
        return 512u;
    default:
        return 512u;
    }
}

[[nodiscard]] inline bool wants_human(const Options& opt) noexcept {
    return opt.human_units != 0;
}

} // namespace pbsd::userland::sbin::df
