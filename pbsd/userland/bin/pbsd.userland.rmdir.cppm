module;
#include <cstddef>

export module pbsd.userland.rmdir;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/rmdir/rmdir.c — option parse + parent-path trim helpers.
export namespace pbsd::userland::bin::rmdir {

struct Options {
    bool parents{false};
    bool verbose{false};
};

[[nodiscard]] inline char* cstrrchr(char* s, char ch) noexcept {
    if (s == nullptr) {
        return nullptr;
    }
    char* last = nullptr;
    for (; *s != '\0'; ++s) {
        if (*s == ch) {
            last = s;
        }
    }
    return last;
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    while (i < argc && argv[i] != nullptr && argv[i][0] == '-') {
        const char* flag = argv[i];
        if (flag[1] == '\0') {
            return result_err<Options>(Status::Invalid);
        }
        for (int j = 1; flag[j] != '\0'; ++j) {
            switch (flag[j]) {
            case 'p':
                opt.parents = true;
                break;
            case 'v':
                opt.verbose = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
        ++i;
    }
    if (i >= argc || argv[i] == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

[[nodiscard]] inline std::size_t trim_trailing_slashes(char* path) noexcept {
    if (path == nullptr) {
        return 0;
    }
    std::size_t len = hosted::cstrlen(path);
    if (len == 0) {
        return 0;
    }
    while (len > 1 && path[len - 1] == '/') {
        path[--len] = '\0';
    }
    return len;
}

/// Pop one directory component (mutate path). Returns false if at root/empty.
[[nodiscard]] inline bool pop_parent(char* path) noexcept {
    if (path == nullptr) {
        return false;
    }
    trim_trailing_slashes(path);
    char* slash = cstrrchr(path, '/');
    if (slash == nullptr || slash == path) {
        return false;
    }
    *slash = '\0';
    trim_trailing_slashes(path);
    return path[0] != '\0';
}

} // namespace pbsd::userland::bin::rmdir
