module;

export module pbsd.userland.libc.gen.dirname;

export import pbsd.core;

/// dirname scaffold from hbsd/src/lib/libc/gen/dirname.c
export namespace pbsd::userland::libc {

/// Returns pointer into mutable path buffer (POSIX dirname semantics scaffold).
[[nodiscard]] inline char* dirname(char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return path;
    }
    char* last_slash = nullptr;
    for (char* p = path; *p != '\0'; ++p) {
        if (*p == '/') {
            last_slash = p;
        }
    }
    if (last_slash == nullptr) {
        path[0] = '.';
        path[1] = '\0';
        return path;
    }
    if (last_slash == path) {
        path[1] = '\0';
        return path;
    }
    *last_slash = '\0';
    return path;
}

} // namespace pbsd::userland::libc
