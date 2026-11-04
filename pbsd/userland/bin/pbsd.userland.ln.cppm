module;
#include <cstddef>

export module pbsd.userland.ln;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/ln/ln.c — option parsing and link target helpers.
export namespace pbsd::userland::bin::ln {

struct Options {
    bool force{false};
    bool interactive{false};
    bool no_dereference{false}; // -h / -n
    bool physical{true};        // -P default
    bool symbolic{false};
    bool verbose{false};
    bool warn_missing{false};   // -w
    bool remove_empty_dir{false}; // -F with -s
    bool link_program{false};   // invoked as "link"
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                const char* progname) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    if (progname != nullptr && hosted::cstrcmp(progname, "link") == 0) {
        opt.link_program = true;
        if (argc != 3) {
            return result_err<Options>(Status::Invalid);
        }
        return result_ok(opt);
    }

    int i = 1;
    while (i < argc && argv[i] != nullptr && argv[i][0] == '-') {
        const char* flag = argv[i];
        if (flag[1] == '\0') {
            return result_err<Options>(Status::Invalid);
        }
        for (int j = 1; flag[j] != '\0'; ++j) {
            switch (flag[j]) {
            case 'F':
                opt.remove_empty_dir = true;
                break;
            case 'L':
                opt.physical = false;
                break;
            case 'P':
                opt.physical = true;
                break;
            case 'f':
                opt.force = true;
                opt.interactive = false;
                opt.warn_missing = false;
                break;
            case 'h':
            case 'n':
                opt.no_dereference = true;
                break;
            case 'i':
                opt.interactive = true;
                opt.force = false;
                break;
            case 's':
                opt.symbolic = true;
                break;
            case 'v':
                opt.verbose = true;
                break;
            case 'w':
                opt.warn_missing = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
        ++i;
    }
    if (i >= argc) {
        return result_err<Options>(Status::Invalid);
    }
    if (opt.remove_empty_dir && !opt.symbolic) {
        opt.remove_empty_dir = false;
    }
    if (opt.remove_empty_dir && !opt.interactive) {
        opt.force = true;
        opt.warn_missing = false;
    }
    return result_ok(opt);
}

[[nodiscard]] inline char link_char(bool symbolic) noexcept {
    return symbolic ? '-' : '=';
}

[[nodiscard]] inline const char* basename_component(const char* path) noexcept {
    if (path == nullptr) {
        return nullptr;
    }
    const char* last = path;
    for (const char* p = path; *p != '\0'; ++p) {
        if (*p == '/') {
            last = p + 1;
        }
    }
    return last;
}

[[nodiscard]] inline bool basenames_match(const char* a, const char* b) noexcept {
    return hosted::cstrcmp(basename_component(a), basename_component(b)) == 0;
}

/// samedirent basename comparison from ln.c.
[[nodiscard]] inline bool same_dir_entry_names(const char* path1,
                                               const char* path2) noexcept {
    if (path1 == nullptr || path2 == nullptr) {
        return false;
    }
    if (hosted::cstrcmp(path1, path2) == 0) {
        return true;
    }
    return basenames_match(path1, path2);
}

[[nodiscard]] inline bool should_append_to_dir(const char* target, bool isdir,
                                               bool hflag, bool fflag) noexcept {
    if (target == nullptr) {
        return false;
    }
    const char* p = target;
    const char* slash = nullptr;
    for (; *p != '\0'; ++p) {
        if (*p == '/') {
            slash = p;
        }
    }
    p = (slash != nullptr) ? slash + 1 : target;
    if (p[0] == '\0' || (p[0] == '.' && p[1] == '\0')) {
        return true;
    }
    (void)hflag;
    if (!fflag && isdir) {
        return true;
    }
    return isdir;
}

[[nodiscard]] inline bool join_target_path(char* out, std::size_t out_len,
                                           const char* dir, const char* source) noexcept {
    if (out == nullptr || dir == nullptr || source == nullptr || out_len == 0) {
        return false;
    }
    const char* base = basename_component(source);
    std::size_t dlen = hosted::cstrlen(dir);
    std::size_t blen = hosted::cstrlen(base);
    if (dlen + 1 + blen + 1 > out_len) {
        return false;
    }
    std::size_t pos = 0;
    for (std::size_t i = 0; i < dlen; ++i) {
        out[pos++] = dir[i];
    }
    if (dlen == 0 || dir[dlen - 1] != '/') {
        out[pos++] = '/';
    }
    for (std::size_t i = 0; i < blen; ++i) {
        out[pos++] = base[i];
    }
    out[pos] = '\0';
    return true;
}

} // namespace pbsd::userland::bin::ln
