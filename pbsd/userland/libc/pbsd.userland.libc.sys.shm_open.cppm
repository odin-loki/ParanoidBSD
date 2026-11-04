module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.shm_open;

export import pbsd.core;

/// shm_open from hbsd/src/lib/libc/sys/shm_open.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status shm_open_name(const char* name) noexcept {
    if (name == nullptr || name[0] == '\0') return Status::Invalid;
    return Status::Ok;
}

} // namespace pbsd::userland::libc
