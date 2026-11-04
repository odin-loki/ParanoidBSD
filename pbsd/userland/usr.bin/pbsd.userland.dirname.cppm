module;
#include <cstddef>

export module pbsd.userland.dirname;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/dirname/dirname.c — path parent extraction (logic-only).
export namespace pbsd::userland::usr_bin::dirname {

[[nodiscard]] inline char* dirname_component(char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return path;
    }
    char* end = path + hosted::cstrlen(path) - 1;
    while (end > path && *end == '/') {
        *end-- = '\0';
    }
    if (end == path && *end == '/') {
        return path;
    }
    while (end > path && *end != '/') {
        --end;
    }
    if (end == path) {
        if (*end == '/') {
            *(end + 1) = '\0';
        } else {
            *end = '.';
            *(end + 1) = '\0';
        }
        return path;
    }
    *end = '\0';
    while (end > path && *(end - 1) == '/') {
        --end;
    }
    *end = '\0';
    if (path[0] == '\0') {
        path[0] = '.';
        path[1] = '\0';
    }
    return path;
}

[[nodiscard]] inline Result<int> parse_args(int argc, char* const* argv,
                                            int& optind_out) noexcept {
    if (argv == nullptr) {
        return result_err<int>(Status::Invalid);
    }
    int i = 1;
    while (i < argc && argv[i] != nullptr && argv[i][0] == '-') {
        if (argv[i][1] != '\0') {
            return result_err<int>(Status::Invalid);
        }
        ++i;
    }
    optind_out = i;
    if (i >= argc) {
        return result_err<int>(Status::Invalid);
    }
    return result_ok(argc - i);
}

} // namespace pbsd::userland::usr_bin::dirname
