module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.rs;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/rs/rs.cc — reshape data arrays.
export namespace pbsd::userland::usr_bin::rs {

enum Flag : std::uint32_t {
    Transpose = 0x000001U,
    MTranspose = 0x000002U,
    OnePerLine = 0x000004U,
    OneIsepOnly = 0x000010U,
    OneOsepOnly = 0x000020U,
    NoTrimEndCol = 0x000040U,
    Squeeze = 0x000100U,
    ShapeOnly = 0x000200U,
    DetailShape = 0x000400U,
    RightAdjust = 0x001000U,
    NullPad = 0x002000U,
    Recycle = 0x004000U,
    SkipPrint = 0x010000U,
    IColBounds = 0x020000U,
    OColBounds = 0x040000U,
    OnePerChar = 0x100000U,
    NoArgs = 0x200000U,
};

struct Options {
    std::uint32_t flags{0};
    char isep{' '};
    char osep{' '};
    std::size_t owidth{80};
    std::size_t gutter{2};
};

[[nodiscard]] inline Options default_no_args() noexcept {
    Options opt{};
    opt.flags = Flag::NoArgs | Flag::Transpose;
    return opt;
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argc <= 1) {
        opt = default_no_args();
        optind_out = 1;
        return result_ok(opt);
    }
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
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'T':
                opt.flags |= Flag::MTranspose;
                [[fallthrough]];
            case 't':
                opt.flags |= Flag::Transpose;
                break;
            case 'c':
                opt.flags |= Flag::OneIsepOnly;
                [[fallthrough]];
            case 's':
                opt.isep = '\t';
                break;
            case 'C':
                opt.flags |= Flag::OneOsepOnly;
                [[fallthrough]];
            case 'S':
                opt.osep = '\t';
                break;
            case '1':
                opt.flags |= Flag::OnePerLine;
                break;
            case 'H':
                opt.flags |= Flag::RightAdjust;
                break;
            case 'h':
                opt.flags |= Flag::RightAdjust;
                break;
            case 'n':
                opt.flags |= Flag::NoTrimEndCol;
                break;
            case 'z':
                opt.flags |= Flag::NullPad;
                break;
            default:
                break;
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline bool accepts_flag(char c) noexcept {
    const char* allowed = "C::EG:HK:S::Tc::eg:hjk:mns::tw:yz";
    for (const char* p = allowed; *p; ++p) {
        if (*p == c) {
            return true;
        }
    }
    return false;
}

} // namespace pbsd::userland::usr_bin::rs
