module;

#include <cstddef>

export module pbsd.userland.libc.gen.getlogin;

export import pbsd.core;

/// getlogin_r scaffold from hbsd/src/lib/libc/gen/getlogin.c
export namespace pbsd::userland::libc {

inline constexpr std::size_t kLoginNameMax = 32;

[[nodiscard]] inline StatusOnly getlogin_r(char* name, std::size_t namelen) noexcept {
    if (name == nullptr || namelen == 0) {
        return status_err(Status::Invalid);
    }
    // Hosted PBSD scaffold: empty login until sysctl wired.
    if (namelen < 1) {
        return status_err(Status::NoMemory);
    }
    name[0] = '\0';
    return status_ok();
}

[[nodiscard]] inline const char* getlogin() noexcept {
    return "";
}

} // namespace pbsd::userland::libc
