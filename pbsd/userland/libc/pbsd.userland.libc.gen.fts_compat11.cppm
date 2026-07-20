module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.fts_compat11;

export import pbsd.core;

/// fts_compat11 from hbsd/src/lib/libc/gen/fts_compat11.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status fts_compat11_path(const char* path) noexcept { return path == nullptr ? Status::Invalid : Status::Ok; }

} // namespace pbsd::userland::libc
