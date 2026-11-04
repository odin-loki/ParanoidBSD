module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.fts_compat;

export import pbsd.core;

/// fts_compat from hbsd/src/lib/libc/gen/fts_compat.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fts_compat_path(const char* path) noexcept { return path == nullptr ? Status::Invalid : Status::Ok; }

} // namespace pbsd::userland::libc
