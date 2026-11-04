module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.writev;

export import pbsd.core;

/// writev from hbsd/src/lib/libc/sys/writev.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Result<std::size_t> writev_count(std::size_t n) noexcept { return result_ok(n); }

} // namespace pbsd::userland::libc
