export module pbsd.userland.env;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/env/env.c — NAME=value split + -i/-u flags.
export namespace pbsd::userland::usr_bin::env {

struct Options {
    bool clear_env{false}; // -i
    bool ignore_env{false}; // synonym
};

struct Assignment {
    const char* name{nullptr};
    const char* value{nullptr};
};

[[nodiscard]] inline bool is_assignment(const char* s) noexcept {
    if (s == nullptr || s[0] == '\0' || s[0] == '=') {
        return false;
    }
    for (const char* p = s; *p; ++p) {
        if (*p == '=') {
            return p != s;
        }
    }
    return false;
}

/// Split "NAME=value" without mutating; value points into s after '='.
[[nodiscard]] inline Result<Assignment> split_assignment(const char* s) noexcept {
    if (!is_assignment(s)) {
        return result_err<Assignment>(Status::Invalid);
    }
    Assignment a{};
    a.name = s;
    for (const char* p = s; *p; ++p) {
        if (*p == '=') {
            a.value = p + 1;
            // name length is (p - s); callers copy if needed
            (void)p;
            break;
        }
    }
    return result_ok(a);
}

[[nodiscard]] inline Result<Options> parse_options(int argc, char* const* argv,
                                                   int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-' && argv[i][1] != '\0'; ++i) {
        if (argv[i][1] == '-' && argv[i][2] == '\0') {
            ++i;
            break;
        }
        for (const char* p = argv[i] + 1; *p; ++p) {
            switch (*p) {
            case 'i':
                opt.clear_env = true;
                break;
            case 'u':
                if (p[1] == '\0') {
                    ++i; // unset name follows
                }
                break;
            case '0':
            case 'P':
            case 'S':
            case 'v':
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::env
