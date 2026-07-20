module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.eui64;

export import pbsd.core;

/// eui64 from hbsd/src/lib/libc/net/eui64.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status eui64_parse(const char* s) noexcept { return s ? Status::Ok : Status::Invalid; }

} // namespace pbsd::userland::libc
