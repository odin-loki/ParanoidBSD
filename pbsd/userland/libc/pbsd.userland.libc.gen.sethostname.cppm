module;
#include <cstddef>

export module pbsd.userland.libc.gen.sethostname;

export import pbsd.core;

/// sethostname scaffold from hbsd/src/lib/libc/gen/sethostname.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline StatusOnly sethostname(const char* name, std::size_t namelen) noexcept {
    if (name == nullptr || namelen == 0) {
        return status_err(Status::Invalid);
    }
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc
