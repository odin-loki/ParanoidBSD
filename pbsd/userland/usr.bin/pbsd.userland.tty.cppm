module;

export module pbsd.userland.tty;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/tty/tty.c — terminal name query.
export namespace pbsd::userland::usr_bin::tty {

inline constexpr const char* kOptString = "s";

struct Options {
    bool silent{false};
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
            if (arg[j] == 's') {
                opt.silent = true;
            } else {
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline int exit_for_tty(const char* ttyname) noexcept {
    return ttyname != nullptr ? 0 : 1;
}

} // namespace pbsd::userland::usr_bin::tty
