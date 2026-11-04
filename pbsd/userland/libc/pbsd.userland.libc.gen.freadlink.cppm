module;

#include <cstddef>

export module pbsd.userland.libc.gen.freadlink;

/// freadlink concept from hbsd/src/lib/libc/gen/freadlink.c (logic-only)
export namespace pbsd::userland::libc {

[[nodiscard]] inline std::size_t bounded_readlink_copy(char* buf, std::size_t bufsiz,
                                                         const char* target) noexcept {
    if (buf == nullptr || bufsiz == 0 || target == nullptr) {
        return 0;
    }
    std::size_t i = 0;
    while (target[i] != '\0' && i + 1 < bufsiz) {
        buf[i] = target[i];
        ++i;
    }
    buf[i] = '\0';
    return i;
}

} // namespace pbsd::userland::libc
