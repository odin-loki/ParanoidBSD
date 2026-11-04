module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.utxdb;

export import pbsd.core;

/// utxdb from hbsd/src/lib/libc/gen/utxdb.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status utxdb_path(const char* path) noexcept { return path == nullptr ? Status::Invalid : Status::Ok; }

} // namespace pbsd::userland::libc
