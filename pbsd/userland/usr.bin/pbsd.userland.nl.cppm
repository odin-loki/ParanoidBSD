module;
#include <cstddef>

export module pbsd.userland.nl;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/nl/nl.c — numbering mode helpers (logic-only).
export namespace pbsd::userland::usr_bin::nl {

enum class NumberingType : unsigned char {
    All,
    NonEmpty,
    None,
    Regex,
};

struct Options {
    NumberingType body{NumberingType::NonEmpty};
    NumberingType header{NumberingType::None};
    NumberingType footer{NumberingType::None};
    const char* separator{"\t"};
    int width{6};
    bool no_renumber{false};
};

[[nodiscard]] inline Result<NumberingType> type_from_char(char c) noexcept {
    switch (c) {
    case 'a':
        return result_ok(NumberingType::All);
    case 't':
        return result_ok(NumberingType::NonEmpty);
    case 'n':
        return result_ok(NumberingType::None);
    case 'p':
        return result_ok(NumberingType::Regex);
    default:
        return result_err<NumberingType>(Status::Invalid);
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
        const char* arg = argv[i];
        if (arg[1] == 'b' && arg[2] != '\0') {
            const auto t = type_from_char(arg[2]);
            if (!t.has_value()) {
                return result_err<Options>(Status::Invalid);
            }
            opt.body = t.value;
            continue;
        }
        if (arg[1] == 'h' && arg[2] != '\0') {
            const auto t = type_from_char(arg[2]);
            if (!t.has_value()) {
                return result_err<Options>(Status::Invalid);
            }
            opt.header = t.value;
            continue;
        }
        if (arg[1] == 'f' && arg[2] != '\0') {
            const auto t = type_from_char(arg[2]);
            if (!t.has_value()) {
                return result_err<Options>(Status::Invalid);
            }
            opt.footer = t.value;
            continue;
        }
        if (arg[1] == 's' && arg[2] != '\0') {
            opt.separator = arg + 2;
            continue;
        }
        if (arg[1] == 'w' && arg[2] != '\0') {
            opt.width = 0;
            for (const char* p = arg + 2; *p >= '0' && *p <= '9'; ++p) {
                opt.width = opt.width * 10 + (*p - '0');
            }
            continue;
        }
        if (hosted::cstrcmp(arg, "-p") == 0) {
            opt.no_renumber = true;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::nl
