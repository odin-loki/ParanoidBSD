module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.getdomainname;

export import pbsd.core;

/// getdomainname from hbsd/src/lib/libc/gen/getdomainname.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status getdomainname_buf(char* name, std::size_t len) noexcept {
    if (name == nullptr || len == 0) {
        return Status::Invalid;
    }
    name[0] = '\0';
    return Status::Ok;
}

} // namespace pbsd::userland::libc
