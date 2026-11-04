module;
#include <cstddef>

export module pbsd.userland.basename;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/basename/basename.c — flag/suffix parse (logic-only).
export namespace pbsd::userland::usr_bin::basename {

struct Options {
    bool all{false};
    const char* suffix{nullptr};
    std::size_t suffix_len{0};
};

[[nodiscard]] inline char* last_slash(char* path) noexcept {
    if (path == nullptr) {
        return nullptr;
    }
    char* last = nullptr;
    for (char* p = path; *p != '\0'; ++p) {
        if (*p == '/') {
            last = p;
        }
    }
    return last;
}

[[nodiscard]] inline const char* basename_component(char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return path;
    }
    char* slash = last_slash(path);
    return slash != nullptr ? slash + 1 : path;
}

[[nodiscard]] inline void strip_suffix(char* p, const char* suffix,
                                       std::size_t suffixlen) noexcept {
    if (p == nullptr || suffix == nullptr || suffixlen == 0) {
        return;
    }
    std::size_t plen = hosted::cstrlen(p);
    if (plen <= suffixlen) {
        return;
    }
    char* q = p + plen - suffixlen;
    if (q > p && hosted::cstrcmp(q, suffix) == 0) {
        *q = '\0';
    }
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                  int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    while (i < argc && argv[i] != nullptr && argv[i][0] == '-') {
        const char* flag = argv[i];
        if (flag[1] == 'a' && flag[2] == '\0') {
            opt.all = true;
            ++i;
            continue;
        }
        if (flag[1] == 's' && flag[2] != '\0') {
            opt.suffix = flag + 2;
            opt.suffix_len = hosted::cstrlen(opt.suffix);
            ++i;
            continue;
        }
        if (flag[1] == 's' && flag[2] == '\0') {
            if (i + 1 >= argc || argv[i + 1] == nullptr) {
                return result_err<Options>(Status::Invalid);
            }
            opt.suffix = argv[i + 1];
            opt.suffix_len = hosted::cstrlen(opt.suffix);
            i += 2;
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    if (i >= argc) {
        return result_err<Options>(Status::Invalid);
    }
    if (!opt.all && opt.suffix == nullptr && (argc - i) >= 2 && argv[i + 1] != nullptr) {
        opt.suffix = argv[i + 1];
        opt.suffix_len = hosted::cstrlen(opt.suffix);
    }
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::basename
