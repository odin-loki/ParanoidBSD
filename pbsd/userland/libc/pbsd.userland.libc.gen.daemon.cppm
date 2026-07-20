module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.daemon;

export import pbsd.core;

/// daemon from hbsd/src/lib/libc/gen/daemon.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status daemon_validate(int nochdir, int noclose) noexcept {
    (void)nochdir;
    (void)noclose;
    return Status::NotImplemented;
}

} // namespace pbsd::userland::libc
