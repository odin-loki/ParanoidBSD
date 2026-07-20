module;
#include <cstddef>

export module pbsd.userland.libc.gen.getcwd;

export import pbsd.core;

/// getcwd scaffold from hbsd/src/lib/libc/gen/getcwd.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline StatusOnly getcwd(char* buf, std::size_t size) noexcept {
    if (buf == nullptr || size == 0) {
        return status_err(Status::Invalid);
    }
    // Hosted scaffold until VFS cwd is wired.
    (void)buf;
    (void)size;
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc
