module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.xprintf_quote;

export import pbsd.core;

/// xprintf_quote from hbsd/src/lib/libc/stdio/xprintf_quote.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status xprintf_quote_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
