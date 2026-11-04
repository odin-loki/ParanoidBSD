module;
#include <cstddef>

export module pbsd.userland.md5;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/sbin/md5/md5.c — Checksum -c/-p/-q
export namespace pbsd::userland::sbin::md5 {

struct Options {
    bool check{false};
    bool quiet{false};
    bool pseudo{false};
};

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
            case 'c':
                opt.check = true;
                break;
            case 'p':
                opt.pseudo = true;
                break;
            case 'q':
                opt.quiet = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

} // namespace pbsd::userland::sbin::md5
