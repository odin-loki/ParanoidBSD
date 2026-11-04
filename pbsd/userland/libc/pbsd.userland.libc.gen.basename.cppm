module;

export module pbsd.userland.libc.gen.basename;

export import pbsd.core;

/// basename scaffold from hbsd/src/lib/libc/gen/basename.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline char* basename(char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return path;
    }
    // Trim trailing slashes (except root).
    char* end = path;
    while (end[1] != '\0') {
        ++end;
    }
    while (end > path && *end == '/') {
        *end = '\0';
        --end;
    }
    char* base = path;
    for (char* p = path; *p != '\0'; ++p) {
        if (*p == '/') {
            base = p + 1;
        }
    }
    return base;
}

} // namespace pbsd::userland::libc
