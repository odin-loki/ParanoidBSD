module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.nscachedcli;

export import pbsd.core;

/// nscachedcli from hbsd/src/lib/libc/net/nscachedcli.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status nscachedcli_validate(const char* arg) noexcept {
    if (arg == nullptr || arg[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
