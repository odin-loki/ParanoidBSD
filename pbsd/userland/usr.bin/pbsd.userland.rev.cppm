module;
#include <cstddef>

export module pbsd.userland.rev;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/rev/rev.c — trivial parse (logic-only).
export namespace pbsd::userland::usr_bin::rev {

struct Options {};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    return result_ok(Options{});
}

[[nodiscard]] inline std::size_t line_length(const char* s) noexcept {
    if (s == nullptr) {
        return 0;
    }
    std::size_t n = 0;
    while (s[n] != '\0' && s[n] != '\n') {
        ++n;
    }
    return n;
}

} // namespace pbsd::userland::usr_bin::rev
