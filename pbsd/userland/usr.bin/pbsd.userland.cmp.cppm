module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.cmp;

export import pbsd.core;
export import pbsd.userland.hosted;
export import pbsd.userland.util.numbers;

/// Port of hbsd/src/usr.bin/cmp/cmp.c — flag/skip helpers (logic-only).
export namespace pbsd::userland::usr_bin::cmp {

struct Options {
    bool print_bytes{false};
    bool verbose{false};
    bool silent{false};
    std::int64_t skip1{0};
    std::int64_t skip2{0};
    std::int64_t limit{-1};
};

[[nodiscard]] inline bool parse_iskipspec(char* spec, std::int64_t& skip1,
                                          std::int64_t& skip2) noexcept {
    if (spec == nullptr) {
        return false;
    }
    char* colon = nullptr;
    for (char* p = spec; *p != '\0'; ++p) {
        if (*p == ':') {
            colon = p;
            *p = '\0';
            break;
        }
    }
    long long n1 = 0;
    if (util::expand_number(spec, &n1) != 0) {
        return false;
    }
    skip1 = static_cast<std::int64_t>(n1);
    if (colon != nullptr) {
        long long n2 = 0;
        if (util::expand_number(colon + 1, &n2) != 0) {
            return false;
        }
        skip2 = static_cast<std::int64_t>(n2);
    } else {
        skip2 = skip1;
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
    for (; i < argc && argv[i] != nullptr; ++i) {
        const char* arg = argv[i];
        if (arg[0] != '-') {
            break;
        }
        if (hosted::cstrcmp(arg, "--print-bytes") == 0) {
            opt.print_bytes = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "--verbose") == 0) {
            opt.verbose = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "--silent") == 0 || hosted::cstrcmp(arg, "--quiet") == 0) {
            opt.silent = true;
            continue;
        }
        if (arg[1] == 'i' && arg[2] != '\0') {
            if (!parse_iskipspec(argv[i] + 2, opt.skip1, opt.skip2)) {
                return result_err<Options>(Status::Invalid);
            }
            continue;
        }
        if (arg[1] == 'i' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            if (!parse_iskipspec(argv[++i], opt.skip1, opt.skip2)) {
                return result_err<Options>(Status::Invalid);
            }
            continue;
        }
        if (arg[1] == 'n' && arg[2] != '\0') {
            long long lim = 0;
            if (util::expand_number(arg + 2, &lim) != 0) {
                return result_err<Options>(Status::Invalid);
            }
            opt.limit = static_cast<std::int64_t>(lim);
            continue;
        }
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'b':
                opt.print_bytes = true;
                break;
            case 'l':
                opt.verbose = true;
                break;
            case 's':
                opt.silent = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::cmp
