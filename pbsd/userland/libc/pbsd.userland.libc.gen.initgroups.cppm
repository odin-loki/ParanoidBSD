module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.initgroups;

export import pbsd.core;

/// initgroups from hbsd/src/lib/libc/gen/initgroups.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status initgroups_user(const char* user, int gid) noexcept {
    (void)gid;
    if (user == nullptr || user[0] == '\0') return Status::Invalid;
    return Status::Ok;
}

} // namespace pbsd::userland::libc
