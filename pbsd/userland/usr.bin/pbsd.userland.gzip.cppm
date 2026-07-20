module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.gzip;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/gzip/gzip.c — flag/level parse (logic-only).
export namespace pbsd::userland::usr_bin::gzip {

struct Options {
    bool decompress{false};
    bool stdout_mode{false};
    bool force{false};
    bool keep{false};
    bool list{false};
    bool test{false};
    bool quiet{false};
    bool verbose{false};
    int level{6};
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
        if (arg[1] >= '1' && arg[1] <= '9' && arg[2] == '\0') {
            opt.level = arg[1] - '0';
            continue;
        }
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'd':
                opt.decompress = true;
                break;
            case 'c':
                opt.stdout_mode = true;
                break;
            case 'f':
                opt.force = true;
                break;
            case 'k':
                opt.keep = true;
                break;
            case 'l':
                opt.list = true;
                break;
            case 't':
                opt.test = true;
                break;
            case 'q':
                opt.quiet = true;
                break;
            case 'v':
                opt.verbose = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline bool gzip_magic(const std::uint8_t* buf, std::size_t len) noexcept {
    return len >= 2 && buf[0] == 0x1f && buf[1] == 0x8b;
}

} // namespace pbsd::userland::usr_bin::gzip
