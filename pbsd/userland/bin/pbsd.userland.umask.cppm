module;
#include <cstddef>

export module pbsd.userland.umask;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/umask/umask.c — file mode mask parsing (logic-only).
export namespace pbsd::userland::bin::umask {

struct Options {
    bool symbolic{false};
    unsigned mode{0};
    bool has_mode{false};
};

[[nodiscard]] inline int parse_octal_digit(char c) noexcept {
    if (c >= '0' && c <= '7') {
        return c - '0';
    }
    return -1;
}

[[nodiscard]] inline Result<unsigned> parse_octal_mode(const char* s) noexcept {
    if (s == nullptr) {
        return result_err<unsigned>(Status::Invalid);
    }
    if (s[0] == '0' && (s[1] == 'o' || s[1] == 'O')) {
        s += 2;
    }
    unsigned val = 0;
    for (; *s != '\0'; ++s) {
        const int d = parse_octal_digit(*s);
        if (d < 0) {
            return result_err<unsigned>(Status::Invalid);
        }
        val = (val << 3) | static_cast<unsigned>(d);
        if (val > 0777U) {
            return result_err<unsigned>(Status::Invalid);
        }
    }
    return result_ok(val);
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        if (hosted::cstrcmp(argv[i], "-S") == 0) {
            opt.symbolic = true;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    if (argc - i == 1) {
        auto parsed = parse_octal_mode(argv[i]);
        if (parsed.status != Status::Ok) {
            return result_err<Options>(Status::Invalid);
        }
        opt.mode = parsed.value;
        opt.has_mode = true;
    } else if (argc - i > 1) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

} // namespace pbsd::userland::bin::umask
