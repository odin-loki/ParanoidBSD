module;
#include <cstddef>

export module pbsd.userland.cpio;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/cpio/cpio.c — odc/newc mode letters
export namespace pbsd::userland::usr_bin::cpio {

enum class Mode : unsigned char { CopyOut, CopyIn, Pass, Unknown };

struct Options {
    Mode mode{Mode::Unknown};
    bool verbose{false};
    bool create_dirs{false};
};

[[nodiscard]] inline Mode mode_from_letter(char c) noexcept {
    switch (c) {
    case 'o':
        return Mode::CopyOut;
    case 'i':
        return Mode::CopyIn;
    case 'p':
        return Mode::Pass;
    default:
        return Mode::Unknown;
    }
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        for (const char* p = argv[i] + 1; *p; ++p) {
            switch (*p) {
            case 'o':
            case 'i':
            case 'p':
                opt.mode = mode_from_letter(*p);
                break;
            case 'v':
                opt.verbose = true;
                break;
            case 'd':
                opt.create_dirs = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::cpio
