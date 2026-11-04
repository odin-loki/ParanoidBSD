module;

#include <cstddef>
#include <cstring>

export module pbsd.userland.libc.string.strerror;

/// strerror from hbsd/src/lib/libc/string/strerror.c (POSIX errno subset)
export namespace pbsd::userland::libc {

struct ErrnoMessage {
    int code;
    const char* text;
};

inline constexpr ErrnoMessage kErrnoMessages[] = {
    {0, "Success"},
    {1, "Operation not permitted"},
    {2, "No such file or directory"},
    {9, "Bad file descriptor"},
    {12, "Cannot allocate memory"},
    {13, "Permission denied"},
    {22, "Invalid argument"},
    {28, "No space left on device"},
    {38, "Function not implemented"},
};

[[nodiscard]] inline const char* strerror(int errnum) noexcept {
    for (const auto& entry : kErrnoMessages) {
        if (entry.code == errnum) {
            return entry.text;
        }
    }
    return "Unknown error";
}

[[nodiscard]] inline char* strerror_r(int errnum, char* buf, std::size_t buflen) noexcept {
    const char* msg = strerror(errnum);
    if (buf != nullptr && buflen > 0) {
        const std::size_t len = std::strlen(msg);
        const std::size_t copy = (len < buflen - 1) ? len : buflen - 1;
        std::memcpy(buf, msg, copy);
        buf[copy] = '\0';
    }
    return buf;
}

} // namespace pbsd::userland::libc
