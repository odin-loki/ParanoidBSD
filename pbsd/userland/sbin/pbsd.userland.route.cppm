module;

export module pbsd.userland.route;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/sbin/route/route.c — flag parse stubs (logic-only).
export namespace pbsd::userland::sbin::route {

struct Options {
    int af{0};
    int aflen{0};
    bool numeric{false};
    bool quiet{false};
    bool verbose{false};
    bool test{false};
    bool debugonly{false};
    const char* jail_name{nullptr};
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        const char* flag = argv[i];
        for (int j = 1; flag[j] != '\0'; ++j) {
            switch (flag[j]) {
            case '4':
                opt.af = 2; // AF_INET
                opt.aflen = 16;
                break;
            case '6':
                opt.af = 28; // AF_INET6
                opt.aflen = 28;
                break;
            case 'n':
                opt.numeric = true;
                break;
            case 'q':
                opt.quiet = true;
                break;
            case 'v':
                opt.verbose = true;
                break;
            case 't':
                opt.test = true;
                break;
            case 'd':
                opt.debugonly = true;
                break;
            case 'j':
                if (flag[j + 1] == '\0') {
                    if (i + 1 >= argc) {
                        return result_err<Options>(Status::Invalid);
                    }
                    opt.jail_name = argv[++i];
                    goto next_flag;
                }
                opt.jail_name = flag + j + 1;
                goto next_flag;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    next_flag:;
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline int keyword_index(const char* word) noexcept {
    if (word == nullptr) {
        return -1;
    }
    static const char* const kKeywords[] = {
        "add", "change", "chg", "delete", "del", "flush", "get", "monitor", "show", nullptr};
    for (int i = 0; kKeywords[i] != nullptr; ++i) {
        if (hosted::cstrcmp(word, kKeywords[i]) == 0) {
            return i;
        }
    }
    return -1;
}

} // namespace pbsd::userland::sbin::route
