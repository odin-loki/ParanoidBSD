module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.unvis_compat;

export import pbsd.core;

/// unvis_compat from hbsd/src/lib/libc/gen/unvis_compat.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status unvis_compat_str(const char* s) noexcept {
    if (s == nullptr) return Status::Invalid;
    return Status::Ok;
}

} // namespace pbsd::userland::libc
