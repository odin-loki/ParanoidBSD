module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.tls;

export import pbsd.core;

/// tls from hbsd/src/lib/libc/gen/tls.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline void* tls_get_addr() noexcept { return nullptr; }

} // namespace pbsd::userland::libc
