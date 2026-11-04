module;
#include <cstddef>

export module pbsd.userland.libc.gen.gethostname;

export import pbsd.core;

/// gethostname scaffold from hbsd/src/lib/libc/gen/gethostname.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline StatusOnly gethostname(char* name, std::size_t namelen) noexcept {
    if (name == nullptr || namelen == 0) {
        return status_err(Status::Invalid);
    }
    name[0] = '\0';
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc
