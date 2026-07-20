module;

#include <cstddef>

export module pbsd.userland.libc.stdlib.setenv;

import pbsd.userland.libc.string;
import pbsd.userland.libc.stdlib.getenv;

/// setenv/unsetenv name validation from hbsd/src/lib/libc/stdlib/getenv.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline bool valid_env_name(const char* name) noexcept {
    if (name == nullptr || *name == '\0') {
        return false;
    }
    if (*name == '=') {
        return false;
    }
    for (const char* p = name; *p != '\0'; ++p) {
        if (*p == '=') {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline bool valid_env_assignment(const char* assignment) noexcept {
    if (assignment == nullptr) {
        return false;
    }
    const char* eq = assignment;
    while (*eq != '\0' && *eq != '=') {
        ++eq;
    }
    return *eq == '=' && eq != assignment;
}

[[nodiscard]] inline int setenv_slot(char** environ, int capacity, const char* name,
                                     const char* value) noexcept {
    if (!valid_env_name(name) || value == nullptr) {
        return -1;
    }
    for (int i = 0; i < capacity && environ[i] != nullptr; ++i) {
        const char* eq = environ[i];
        while (*eq != '\0' && *eq != '=') {
            ++eq;
        }
        if (*eq != '=') {
            continue;
        }
        const std::size_t nlen = static_cast<std::size_t>(eq - environ[i]);
        if (nlen == strlen(name) && strncmp(environ[i], name, nlen) == 0) {
            return i;
        }
    }
    return -1;
}

} // namespace pbsd::userland::libc
