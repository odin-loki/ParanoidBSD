module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.getnetbydns;

export import pbsd.core;

/// getnetbydns from hbsd/src/lib/libc/net/getnetbydns.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status getnetbydns_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
