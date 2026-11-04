module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.readv;

export import pbsd.core;

/// readv from hbsd/src/lib/libc/sys/readv.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Result<std::size_t> readv_count(std::size_t n) noexcept { return result_ok(n); }

} // namespace pbsd::userland::libc
