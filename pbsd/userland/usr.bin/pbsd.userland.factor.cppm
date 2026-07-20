module;

export module pbsd.userland.factor;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/factor/factor.c — prime factorization scaffold.
export namespace pbsd::userland::usr_bin::factor {

inline constexpr const char* kOptString = "h";

struct Options {
    bool hex{false};
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
        for (int j = 1; arg[j] != '\0'; ++j) {
            if (arg[j] == 'h') {
                opt.hex = true;
            } else {
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline bool is_trivial_factor(unsigned long n) noexcept {
    return n <= 1;
}

[[nodiscard]] inline unsigned long smallest_factor(unsigned long n) noexcept {
    if (n < 2) {
        return n;
    }
    if ((n & 1U) == 0) {
        return 2;
    }
    for (unsigned long f = 3; f * f <= n; f += 2) {
        if (n % f == 0) {
            return f;
        }
    }
    return n;
}

} // namespace pbsd::userland::usr_bin::factor
