module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.wc;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/wc/wc.c — flag parse (logic-only).
export namespace pbsd::userland::usr_bin::wc {

struct Options {
    bool lines{false};
    bool words{false};
    bool chars{false};
    bool multibyte{false};
    bool long_lines{false};
};

struct Counts {
    std::uintmax_t lines{0};
    std::uintmax_t words{0};
    std::uintmax_t chars{0};
    std::uintmax_t long_lines{0};
};

[[nodiscard]] inline void apply_defaults(Options& opt) noexcept {
    if (!opt.lines && !opt.words && !opt.chars && !opt.long_lines) {
        opt.lines = true;
        opt.words = true;
        opt.chars = true;
    }
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        for (const char* p = argv[i] + 1; *p != '\0'; ++p) {
            switch (*p) {
            case 'l':
                opt.lines = true;
                break;
            case 'w':
                opt.words = true;
                break;
            case 'c':
                opt.chars = true;
                opt.multibyte = false;
                break;
            case 'L':
                opt.long_lines = true;
                break;
            case 'm':
                opt.multibyte = true;
                opt.chars = false;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    apply_defaults(opt);
    return result_ok(opt);
}

[[nodiscard]] inline bool is_word_char(unsigned char c) noexcept {
    return c > ' ';
}

[[nodiscard]] inline void count_byte(const unsigned char c, Counts& cts,
                                   bool in_word) noexcept {
    ++cts.chars;
    if (c == '\n') {
        ++cts.lines;
    }
    if (is_word_char(c)) {
        if (!in_word) {
            ++cts.words;
        }
    }
}

} // namespace pbsd::userland::usr_bin::wc
