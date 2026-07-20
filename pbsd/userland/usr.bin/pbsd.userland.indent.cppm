module;
#include <cstddef>

export module pbsd.userland.indent;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/indent/args.c — formatter options (logic-only).
export namespace pbsd::userland::usr_bin::indent {

struct Options {
    int line_length{78};
    int indent_width{8};
    int continuation{8};
    int case_indent{0};
    int comment_dist{1};
    bool proc_calls{false};
    bool leave_comma{false};
};

[[nodiscard]] inline Result<int> parse_positive(const char* s) noexcept {
    if (s == nullptr || *s == '\0') {
        return result_err<int>(Status::Invalid);
    }
    int val = 0;
    for (const char* p = s; *p != '\0'; ++p) {
        if (*p < '0' || *p > '9') {
            return result_err<int>(Status::Invalid);
        }
        val = val * 10 + (*p - '0');
    }
    return result_ok(val);
}

[[nodiscard]] inline Result<Options> apply_flag(char flag, const char* arg,
                                                Options& opt) noexcept {
    switch (flag) {
    case 'l':
        if (arg == nullptr || *arg == '\0') {
            return result_err<Options>(Status::Invalid);
        }
        {
            auto parsed = parse_positive(arg);
            if (parsed.status != Status::Ok) {
                return result_err<Options>(Status::Invalid);
            }
            opt.line_length = parsed.value;
        }
        break;
    case 'i':
        if (arg == nullptr || *arg == '\0') {
            return result_err<Options>(Status::Invalid);
        }
        {
            auto parsed = parse_positive(arg);
            if (parsed.status != Status::Ok) {
                return result_err<Options>(Status::Invalid);
            }
            opt.indent_width = parsed.value;
        }
        break;
    case 'c':
        if (arg == nullptr || *arg == '\0') {
            return result_err<Options>(Status::Invalid);
        }
        {
            auto parsed = parse_positive(arg);
            if (parsed.status != Status::Ok) {
                return result_err<Options>(Status::Invalid);
            }
            opt.continuation = parsed.value;
        }
        break;
    case 'C':
        if (arg == nullptr || *arg == '\0') {
            return result_err<Options>(Status::Invalid);
        }
        {
            auto parsed = parse_positive(arg);
            if (parsed.status != Status::Ok) {
                return result_err<Options>(Status::Invalid);
            }
            opt.case_indent = parsed.value;
        }
        break;
    case 'd':
        if (arg == nullptr || *arg == '\0') {
            return result_err<Options>(Status::Invalid);
        }
        {
            auto parsed = parse_positive(arg);
            if (parsed.status != Status::Ok) {
                return result_err<Options>(Status::Invalid);
            }
            opt.comment_dist = parsed.value;
        }
        break;
    case 'p':
        if (arg != nullptr && arg[0] == 'c' && arg[1] == 's') {
            opt.proc_calls = true;
        }
        break;
    case 'm':
        opt.leave_comma = true;
        break;
    default:
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
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
        if (arg[1] == '\0') {
            return result_err<Options>(Status::Invalid);
        }
        const char* val = (arg[2] != '\0') ? arg + 2 : nullptr;
        if (val == nullptr && i + 1 < argc) {
            val = argv[++i];
        }
        auto applied = apply_flag(arg[1], val, opt);
        if (applied.status != Status::Ok) {
            return result_err<Options>(Status::Invalid);
        }
    }
    optind_out = i;
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::indent
