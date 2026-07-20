module;
#include <cstddef>

export module pbsd.userland.spell;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/spell/spell.c — word validation (logic-only).
export namespace pbsd::userland::usr_bin::spell {

struct Options {
    bool british{false};
    bool stop_list{false};
    const char* dictionary{nullptr};
};

[[nodiscard]] inline bool is_alpha_char(char c) noexcept {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '\'' || c == '-';
}

[[nodiscard]] inline bool valid_word(const char* word) noexcept {
    if (word == nullptr || word[0] == '\0') {
        return false;
    }
    for (const char* p = word; *p != '\0'; ++p) {
        if (!is_alpha_char(*p)) {
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
        if (hosted::cstrcmp(arg, "-b") == 0) {
            opt.british = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-x") == 0) {
            opt.stop_list = true;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    if (argc - i == 1) {
        opt.dictionary = argv[i];
    } else if (argc - i > 1) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::spell
