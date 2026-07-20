module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.gethostbydns;

export import pbsd.core;

/// gethostbydns from hbsd/src/lib/libc/net/gethostbydns.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status gethostbydns_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
