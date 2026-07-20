module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.getsubopt;

export import pbsd.core;

/// getsubopt from hbsd/src/lib/libc/stdlib/getsubopt.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status getsubopt_option(char* const* options) noexcept { return options ? Status::Ok : Status::Invalid; }

} // namespace pbsd::userland::libc
