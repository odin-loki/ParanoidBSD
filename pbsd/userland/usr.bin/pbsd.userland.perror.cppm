module;
#include <cstddef>

export module pbsd.userland.perror;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/perror/perror.c — errno lookup (logic-only).
export namespace pbsd::userland::usr_bin::perror {

struct Options {
    long errnum{};
};

[[nodiscard]] inline Result<long> parse_errno_arg(const char* s) noexcept {
    if (s == nullptr || *s == '\0') {
        return result_err<long>(Status::Invalid);
    }
    long val = 0;
    int base = 10;
    const char* p = s;
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
        base = 16;
        p += 2;
    }
    if (*p == '\0') {
        return result_err<long>(Status::Invalid);
    }
    for (; *p != '\0'; ++p) {
        int digit = -1;
        if (*p >= '0' && *p <= '9') {
            digit = *p - '0';
        } else if (base == 16 && *p >= 'a' && *p <= 'f') {
            digit = 10 + (*p - 'a');
        } else if (base == 16 && *p >= 'A' && *p <= 'F') {
            digit = 10 + (*p - 'A');
        } else {
            return result_err<long>(Status::Invalid);
        }
        val = val * base + digit;
    }
    return result_ok(val);
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv) noexcept {
    if (argv == nullptr || argc != 2) {
        return result_err<Options>(Status::Invalid);
    }
    auto parsed = parse_errno_arg(argv[1]);
    if (parsed.status != Status::Ok) {
        return result_err<Options>(Status::Invalid);
    }
    Options opt{};
    opt.errnum = parsed.value;
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::perror
