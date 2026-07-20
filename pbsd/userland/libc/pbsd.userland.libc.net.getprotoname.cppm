module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.getprotoname;

export import pbsd.core;

/// getprotoname from hbsd/src/lib/libc/net/getprotoname.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status getprotoname_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
