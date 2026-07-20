module;
#include <cstddef>
#include <cwchar>

export module pbsd.userland.paste;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/paste/paste.c — flag parse (logic-only).
export namespace pbsd::userland::usr_bin::paste {

struct Options {
    bool sequential{false};
    wchar_t delim{L'\t'};
    int delim_count{1};
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
        if (hosted::cstrcmp(arg, "-s") == 0) {
            opt.sequential = true;
            continue;
        }
        if (arg[1] == 'd' && arg[2] != '\0') {
            opt.delim = static_cast<wchar_t>(arg[2]);
            opt.delim_count = 1;
            continue;
        }
        if (arg[1] == 'd' && arg[2] == '\0') {
            if (i + 1 >= argc || argv[i + 1][0] == '\0') {
                return result_err<Options>(Status::Invalid);
            }
            opt.delim = static_cast<wchar_t>(argv[++i][0]);
            opt.delim_count = 1;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    if (i >= argc) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::paste
