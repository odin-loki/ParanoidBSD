module;
#include <cstddef>

export module pbsd.userland.tset;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tset/tset.c — terminal init options (logic-only).
export namespace pbsd::userland::usr_bin::tset {

struct Options {
    bool query{false};
    bool quiet{false};
    bool reset{false};
    const char* term{nullptr};
};

[[nodiscard]] inline bool valid_term_name(const char* term) noexcept {
    if (term == nullptr || term[0] == '\0') {
        return false;
    }
    for (const char* p = term; *p != '\0'; ++p) {
        const char c = *p;
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '-' || c == '.')) {
            return false;
        }
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
        if (hosted::cstrcmp(arg, "-Q") == 0) {
            opt.query = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-q") == 0) {
            opt.quiet = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-r") == 0) {
            opt.reset = true;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    if (argc - i == 1) {
        if (!valid_term_name(argv[i])) {
            return result_err<Options>(Status::Invalid);
        }
        opt.term = argv[i];
    } else if (argc - i > 1) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::tset
