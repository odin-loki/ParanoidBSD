module;
#include <cstddef>

export module pbsd.userland.seq;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/seq/seq.c — flag/format helpers (logic-only).
export namespace pbsd::userland::usr_bin::seq {

struct Options {
    const char* format{nullptr};
    const char* separator{"\n"};
    const char* terminator{nullptr};
    bool equal_width{false};
    char pad{'0'};
};

[[nodiscard]] inline bool is_numeric_token(const char* s) noexcept {
    if (s == nullptr || *s == '\0') {
        return false;
    }
    if (*s == '-' || *s == '+') {
        ++s;
    }
    if (*s == '\0') {
        return false;
    }
    bool saw_digit = false;
    for (; *s != '\0'; ++s) {
        if (*s >= '0' && *s <= '9') {
            saw_digit = true;
        } else if (*s == '.' || *s == 'e' || *s == 'E') {
            continue;
        } else {
            return false;
        }
    }
    return saw_digit;
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
        if (hosted::cstrcmp(arg, "--equal-width") == 0 || hosted::cstrcmp(arg, "-w") == 0) {
            opt.equal_width = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-f") == 0 || hosted::cstrcmp(arg, "--format") == 0) {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.format = argv[++i];
            continue;
        }
        if (hosted::cstrcmp(arg, "-s") == 0 || hosted::cstrcmp(arg, "--separator") == 0) {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.separator = argv[++i];
            continue;
        }
        if (hosted::cstrcmp(arg, "-t") == 0 || hosted::cstrcmp(arg, "--terminator") == 0) {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.terminator = argv[++i];
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline int arg_count(int argc, int optind) noexcept {
    return argc - optind;
}

} // namespace pbsd::userland::usr_bin::seq
