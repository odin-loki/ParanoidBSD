module;
#include <cstddef>

export module pbsd.userland.libc.gen.ttyname;

export import pbsd.core;

/// ttyname scaffold from hbsd/src/lib/libc/gen/ttyname.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline StatusOnly ttyname_r(int /*fd*/, char* buf, std::size_t buflen) noexcept {
    if (buf == nullptr || buflen == 0) {
        return status_err(Status::Invalid);
    }
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc
