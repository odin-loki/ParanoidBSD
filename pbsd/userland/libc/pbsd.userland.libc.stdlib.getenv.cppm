module;

#include <cstddef>

export module pbsd.userland.libc.stdlib.getenv;

import pbsd.userland.libc.string;

/// getenv scan from hbsd/src/lib/libc/stdlib/getenv.c (logic-only)
export namespace pbsd::userland::libc {

[[nodiscard]] inline const char* lookup_env(char* const* environ, const char* name) noexcept {
    if (environ == nullptr || name == nullptr) {
        return nullptr;
    }
    const std::size_t nlen = strlen(name);
    for (char* const* ep = environ; *ep != nullptr; ++ep) {
        const char* eq = *ep;
        while (*eq != '\0' && *eq != '=') {
            ++eq;
        }
        if (*eq != '=') {
            continue;
        }
        if (static_cast<std::size_t>(eq - *ep) == nlen &&
            strncmp(*ep, name, nlen) == 0) {
            return eq + 1;
        }
    }
    return nullptr;
}

} // namespace pbsd::userland::libc
