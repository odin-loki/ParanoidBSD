module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.setdomainname;

export import pbsd.core;

/// setdomainname from hbsd/src/lib/libc/gen/setdomainname.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status setdomainname_buf(const char* name, std::size_t len) noexcept {
    if (name == nullptr || len == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
