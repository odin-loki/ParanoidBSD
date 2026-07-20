module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.ftw_compat11;

export import pbsd.core;

/// ftw_compat11 from hbsd/src/lib/libc/gen/ftw_compat11.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status ftw_compat11_path(const char* path) noexcept { return path == nullptr ? Status::Invalid : Status::Ok; }

} // namespace pbsd::userland::libc
