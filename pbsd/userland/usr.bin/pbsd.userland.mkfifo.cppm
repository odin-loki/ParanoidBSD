module;
#include <cstddef>

export module pbsd.userland.mkfifo;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkfifo/mkfifo.c — mode helpers (logic-only).
export namespace pbsd::userland::usr_bin::mkfifo {

inline constexpr unsigned kBaseMode = 0666U;

struct Options {
    bool set_mode{false};
    const char* mode_string{nullptr};
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
        if (hosted::cstrcmp(arg, "-m") == 0) {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.set_mode = true;
            opt.mode_string = argv[++i];
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    if (argc - i < 1) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

[[nodiscard]] inline unsigned default_fifo_mode() noexcept {
    return kBaseMode;
}

[[nodiscard]] inline bool is_octal_mode_char(char c) noexcept {
    return c >= '0' && c <= '7';
}

[[nodiscard]] inline Result<unsigned> parse_octal_mode(const char* s) noexcept {
    if (s == nullptr || *s == '\0') {
        return result_err<unsigned>(Status::Invalid);
    }
    unsigned mode = 0;
    for (const char* p = s; *p != '\0'; ++p) {
        if (!is_octal_mode_char(*p)) {
            return result_err<unsigned>(Status::Invalid);
        }
        mode = (mode << 3) + static_cast<unsigned>(*p - '0');
    }
    return result_ok(mode);
}

} // namespace pbsd::userland::usr_bin::mkfifo
