module;
#include <cstddef>

export module pbsd.userland.tsort;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tsort/tsort.c — flag parse (logic-only).
export namespace pbsd::userland::usr_bin::tsort {

struct Options {
    bool debug{false};
    bool long_format{false};
    bool quiet{false};
    const char* input_file{nullptr};
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
        if (hosted::cstrcmp(arg, "-d") == 0) {
            opt.debug = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-l") == 0) {
            opt.long_format = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-q") == 0) {
            opt.quiet = true;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    if (i < argc) {
        if (i + 1 != argc) {
            return result_err<Options>(Status::Invalid);
        }
        opt.input_file = argv[i];
    }
    return result_ok(opt);
}

[[nodiscard]] inline bool is_edge_token(const char* a, const char* b) noexcept {
    return a != nullptr && b != nullptr && a[0] != '\0' && b[0] != '\0';
}

} // namespace pbsd::userland::usr_bin::tsort
