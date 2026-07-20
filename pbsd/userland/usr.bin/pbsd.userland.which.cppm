module;
#include <cstddef>

export module pbsd.userland.which;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/which/which.c — PATH search (logic-only).
export namespace pbsd::userland::usr_bin::which {

struct Options {
    bool all_paths{false};
    bool silent{false};
};

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
            case 'a':
                opt.all_paths = true;
                break;
            case 's':
                opt.silent = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    if (i >= argc) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

[[nodiscard]] inline bool has_slash(const char* name) noexcept {
    if (name == nullptr) {
        return false;
    }
    for (const char* p = name; *p != '\0'; ++p) {
        if (*p == '/') {
            return true;
        }
    }
    return false;
}

/// Split next PATH element; mutates `path` at delimiter.
[[nodiscard]] inline const char* next_path_elem(char*& path) noexcept {
    if (path == nullptr) {
        return nullptr;
    }
    const char* elem = path;
    char* sep = nullptr;
    for (char* p = path; *p != '\0'; ++p) {
        if (*p == ':') {
            sep = p;
            break;
        }
    }
    if (sep != nullptr) {
        *sep = '\0';
        path = sep + 1;
    } else {
        path = nullptr;
    }
    if (elem[0] == '\0') {
        return ".";
    }
    return elem;
}

[[nodiscard]] inline bool join_candidate(char* out, std::size_t cap,
                                         const char* dir,
                                         const char* file) noexcept {
    if (out == nullptr || dir == nullptr || file == nullptr) {
        return false;
    }
    std::size_t pos = 0;
    for (const char* p = dir; *p != '\0'; ++p) {
        if (pos + 2 >= cap) {
            return false;
        }
        out[pos++] = *p;
    }
    if (pos + 2 >= cap) {
        return false;
    }
    out[pos++] = '/';
    for (const char* p = file; *p != '\0'; ++p) {
        if (pos + 1 >= cap) {
            return false;
        }
        out[pos++] = *p;
    }
    out[pos] = '\0';
    return true;
}

} // namespace pbsd::userland::usr_bin::which
